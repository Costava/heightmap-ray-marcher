#include <climits>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "glm/glm.hpp"

#include "stb_image.h"
#include "stb_image_write.h"

#include "AABB.hpp"
#include "ImagePlane.hpp"
#include "Perspective.hpp"
#include "Spherical.hpp"
#include "Orthographic.hpp"

SDL_Window *window = NULL;

// Window content dimensions (pixels)
int screen_width = 800;
int screen_height = 600;

// Horizontal field of view (radians)
double hfov = M_PI / 2.0;

// Normalize heights in heightmap between these bounds
double min_height = 0.0;
double max_height = 10.0;

// For each pixel in the given heightmap image with components RGB,
// the pixel's heightmap value is (rR + gG + bB),
// clamped to range [0.0, 255.0],
// then scaled to range [min_height, max_height]
double lum_r = 0.299;
double lum_g = 0.587;
double lum_b = 0.114;

// BGR888 (R first component in buffer)
const unsigned char *base_heightmap_buf = NULL;
// Array of heights in range [min_height, max_height]
double *heightmap_buf = NULL;
int heightmap_width;
int heightmap_height;

// Array of RGB unsigned char values
const unsigned char *colormap_buf = NULL;
int colormap_width;
int colormap_height;

// Width and height of grid cells of image heightmap
// i.e. how far apart pixels from the image are in world space when rendered
double grid_width = 0.05;

// How far to step at a time when raymarching
double step_dist = 5.0 * grid_width;

// Draw a full image across `cycle_period` number of frames.
int cycle_period = 47;
int cycle = 0;

// Position of camera
glm::dvec3 cam_pos(-5.0, 5.0, 0.0);

// Horizontal angle of camera (rads)
// 0 is looking in direction of positive x axis
// pi / 2 is looking in direction of positive y axis
double hang = -M_PI / 4.0;

// Vertical angle of camera (rads)
// 0 is looking straight up (with positive z axis)
// pi / 2 is looking parallel to xy plane
double vang = M_PI / 2.0;

// Horizontal and vertical mouse sensitivity
double mouse_sens = 1.0;
// Sensitivity when zooming in/out with scroll wheel.
double scroll_sens = 1.0;

// Unit is units/millisecond
double move_speed = 0.05;

bool fullscreen = false;

// How far apart the orthographic rays are
double ortho_width = 2.0 * grid_width;

// The number of frames to render when recording
// (saving frames out to image files).
int recording_frame_count = 200;

#define IMAGEPLANE_PERSPECTIVE  1
#define IMAGEPLANE_SPHERICAL    2
#define IMAGEPLANE_ORTHOGRAPHIC 3
int image_plane = IMAGEPLANE_PERSPECTIVE;

// Background color
Uint8 bg_r = 0;
Uint8 bg_g = 0;
Uint8 bg_b = 0;

template<class T>
static T Clamp(const T value, const T min, const T max) {
	if (value < min) return min;
	if (value > max) return max;

	return value;
}

template<class T>
static T Lerp(const double prop, T v0, T v1) {
	return v0 + prop * (v1 - v0);
}

static double DegreesToRads(const double degrees) {
	return (degrees / 180.0) * M_PI;
}

static double RadsToDegrees(const double rads) {
	return (rads / M_PI) * 180.0;
}

static void SetPixel(
	Uint8 *const framebuf,
	const int x,
	const int y,
	const Uint8 r,
	const Uint8 g,
	const Uint8 b,
	const Uint8 a)
{
	const size_t i = (x + y * screen_width) * 4;

	framebuf[i + 0] = r;
	framebuf[i + 1] = g;
	framebuf[i + 2] = b;
	framebuf[i + 3] = a;
}

// Save given RGBA frame buffer as .png image at given path.
static void SavePNG(Uint8 *framebuf, std::string path) {
	const int code = stbi_write_png(path.c_str(),
		screen_width, screen_height, 4,
		framebuf, screen_width * 4);

	if (code == 0) {
		std::cerr << "Failed to write screenshot to " << path << "\n";
	}
	else {
		std::cout << "Saved screenshot at " << path << "\n";
	}
}

// Update `heightmap_buf` using the current global parameters.
static void UpdateHeightmap() {
	const int num_pixels = heightmap_width * heightmap_height;
	delete[] heightmap_buf;
	heightmap_buf = new double[num_pixels];

	int p = 0;
	for (int i = 0; i < num_pixels * 3; i += 3) {
		const unsigned char r = base_heightmap_buf[i + 0];
		const unsigned char g = base_heightmap_buf[i + 1];
		const unsigned char b = base_heightmap_buf[i + 2];

		const double value = Clamp<double>(
			(lum_r * r) + (lum_g * g) + (lum_b * b),
			0.0, 255.0
		);

		heightmap_buf[p] = (value / 255.0)
		                   * (max_height - min_height) + min_height;
		p += 1;
	}
}

// Read stream until end and update config values
static void ConsumeConfigStream(std::istream &input) {
	bool should_update_heightmap = false;

	std::string next;
	while (input >> next) {
		if (next == "heightmap") {
			std::string path;
			input >> path;

			int n;

			stbi_image_free((void*)base_heightmap_buf);
			base_heightmap_buf = stbi_load(
				path.c_str(), &heightmap_width, &heightmap_height, &n, 3);

			if (base_heightmap_buf == NULL) {
				std::cerr
					<< "Failed to load image for heightmap from "
					<< path << "\n";
				std::exit(1);
			}

			should_update_heightmap = true;

			std::cout << "heightmap " << path << "\n";
		}
		else if (next == "colormap") {
			std::string path;
			input >> path;

			int n;

			stbi_image_free((void*)colormap_buf);
			colormap_buf = stbi_load(
				path.c_str(), &colormap_width, &colormap_height, &n, 4);

			if (colormap_buf == NULL) {
				std::cerr
					<< "Failed to load image for colormap from "
					<< path << "\n";
				std::exit(1);
			}

			std::cout << "colormap " << path << "\n";
		}
		else if (next == "resolution") {
			input >> screen_width >> screen_height;

			if (window != NULL) {
				SDL_SetWindowSize(window, screen_width, screen_height);
			}

			std::cout << "resolution " << screen_width  << " "
			                           << screen_height << "\n";
		}
		else if (next == "hfov") {
			double hfdeg;
			input >> hfdeg;

			hfov = DegreesToRads(hfdeg);
			std::cout << "hfov " << hfdeg << "\n";
		}
		else if (next == "hang") {
			double deg;
			input >> deg;

			hang = DegreesToRads(deg);
			std::cout << "hang " << deg << "\n";
		}
		else if (next == "vang") {
			double deg;
			input >> deg;

			vang = DegreesToRads(deg);
			std::cout << "vang " << deg << "\n";
		}
		else if (next == "pos") {
			input >> cam_pos.x >> cam_pos.y >> cam_pos.z;

			std::cout
				<< "pos "
				<< cam_pos.x << " " << cam_pos.y << " " << cam_pos.z << "\n";
		}
		else if (next == "pos_x") {
			input >> cam_pos.x;
			std::cout << "pos_x " << cam_pos.x << "\n";
		}
		else if (next == "pos_y") {
			input >> cam_pos.y;
			std::cout << "pos_y " << cam_pos.y << "\n";
		}
		else if (next == "pos_z") {
			input >> cam_pos.z;
			std::cout << "pos_z " << cam_pos.z << "\n";
		}
		else if (next == "print_pos") {
			std::cout
				<< "pos "
				<< cam_pos.x << " " << cam_pos.y << " " << cam_pos.z << "\n"
				<< "hang " << RadsToDegrees(hang) << "\n"
				<< "vang " << RadsToDegrees(vang) << "\n";
		}
		else if (next == "min_height") {
			input >> min_height;
			should_update_heightmap = true;
			std::cout << "min_height " << min_height << "\n";
		}
		else if (next == "max_height") {
			input >> max_height;
			should_update_heightmap = true;
			std::cout << "max_height " << max_height << "\n";
		}
		else if (next == "lum") {
			input >> lum_r >> lum_g >> lum_b;
			should_update_heightmap = true;
			std::cout << "lum "
			          << lum_r << " " << lum_g << " " << lum_b << "\n";
		}
		else if (next == "lum_r") {
			input >> lum_r;
			should_update_heightmap = true;
			std::cout << "lum_r " << lum_r << "\n";
		}
		else if (next == "lum_g") {
			input >> lum_g;
			should_update_heightmap = true;
			std::cout << "lum_g " << lum_g << "\n";
		}
		else if (next == "lum_b") {
			input >> lum_b;
			should_update_heightmap = true;
			std::cout << "lum_b " << lum_b << "\n";
		}
		else if (next == "grid_width") {
			input >> grid_width;
			std::cout << "grid_width " << grid_width << "\n";
		}
		else if (next == "ortho_width") {
			input >> ortho_width;
			std::cout << "ortho_width " << ortho_width << "\n";
		}
		else if (next == "step_dist") {
			input >> step_dist;
			std::cout << "step_dist " << step_dist << "\n";
		}
		else if (next == "bg_color") {
			// Read into int intermediaries because
			//  stream does not properly read directly to Uint8.
			int r, g, b;
			input >> r >> g >> b;

			bg_r = (Uint8)r;
			bg_g = (Uint8)g;
			bg_b = (Uint8)b;

			// Need casts because
			//  stream does not print Uint8 as ASCII decimal as expected.
			std::cout
				<< "bg_color "
				<< ((int)bg_r) << " "
				<< ((int)bg_g) << " "
				<< ((int)bg_b) << "\n";
		}
		else if (next == "cycle") {
			input >> cycle_period;
			cycle = 0;
			std::cout << "cycle " << cycle_period << "\n";
		}
		else if (next == "mouse_sens") {
			input >> mouse_sens;
			std::cout << "mouse_sens " << mouse_sens << "\n";
		}
		else if (next == "scroll_sens") {
			input >> scroll_sens;
			std::cout << "scroll_sens " << scroll_sens << "\n";
		}
		else if (next == "move") {
			input >> move_speed;
			std::cout << "move " << move_speed << "\n";
		}
		else if (next == "recording_frame_count") {
			input >> recording_frame_count;
			std::cout
				<< "recording_frame_count " << recording_frame_count << "\n";
		}
		else {
			std::cerr << "WARNING: Unknown identifier: " << next << "\n";
		}
	}

	// Some validation

	if (base_heightmap_buf == NULL) {
		std::cerr << "Must specify heightmap in config\n";
		std::exit(1);
	}

	if (colormap_buf == NULL) {
		std::cerr << "Must specify colormap in config\n";
		std::exit(1);
	}

	const bool dimensions_conflict =
		(heightmap_width  != colormap_width) ||
		(heightmap_height != colormap_height);

	if (dimensions_conflict) {
		std::cerr
			<< "heightmap dimensions (" << heightmap_width
			<< "x" << heightmap_height
			<<  ") must match colormap dimensions ("
			<< colormap_width << "x" << colormap_height << ")\n";

		std::exit(1);
	}

	if (should_update_heightmap) {
		UpdateHeightmap();
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "USAGE: hmap.exe path/to/config.txt\n";
		std::exit(1);
	}

	// Parse input file

	std::ifstream input;
	input.open(argv[1]);

	if (!input.is_open()) {
		std::cerr << "Failed to open input file: " << argv[1] << "\n";
		std::exit(1);
	}

	ConsumeConfigStream(input);

	input.close();

	// Initialize libraries

	class ManageSDL {
	public:
		ManageSDL() {
			if (SDL_Init(SDL_INIT_VIDEO) != 0) {
				std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
				std::exit(1);
			}
		}

		~ManageSDL() { SDL_Quit(); }
	};
	const ManageSDL manage_sdl;

	class ManageTTF {
	public:
		ManageTTF() {
			if (TTF_Init() != 0) {
				std::cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
				std::exit(1);
			}
		}

		~ManageTTF() { TTF_Quit(); }
	};
	const ManageTTF manage_ttf;

	const char font_path[] = "fonts/NotoSansMono-Regular.ttf";
	TTF_Font *const font = TTF_OpenFont(font_path, 12);

	if (font == NULL) {
		std::cerr << "Failed to open font at: " << font_path << "\n";
		std::exit(1);
	}

	window = SDL_CreateWindow(
		"Heightmap Ray Marcher",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_width, screen_height,
		SDL_WINDOW_RESIZABLE);

	if (window == NULL) {
		std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
		std::exit(1);
	}

	SDL_Renderer *renderer =
		SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (renderer == NULL) {
		std::cerr << "Failed to create renderer: " << SDL_GetError() << "\n";
		exit(1);
	}

	SDL_Texture *tex = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ABGR8888,
		SDL_TEXTUREACCESS_STREAMING,
		screen_width, screen_height);

	if (tex == NULL) {
		std::cerr << "Failed to create texture: " << SDL_GetError() << "\n";
		std::exit(1);
	}

	Uint8 *framebuf = new Uint8[screen_width * screen_height * 4];

	std::srand((unsigned)std::time(NULL));

	bool quit = false;
	bool show_fps = false;

	SDL_Surface *fps_surface = NULL;
	SDL_Surface *console_surface = NULL;

	// To avoid the text changing too frequently to be readable,
	//  update the text only every X ms.
	const Uint32 text_surface_rerender_period_ms = 200;
	Uint32 text_surface_rerender_timer_ms = text_surface_rerender_period_ms + 1;

	bool console_active = false;
	std::string console_buf;
	console_buf.assign(" ");

	Uint32 old_time = SDL_GetTicks();// milliseconds
	Uint32 new_time;
	Uint32 delta;
	double ddelta;

	if (SDL_SetRelativeMouseMode(SDL_TRUE)) {
		std::cerr << "Initial SDL_SetRelativeMouseMode failed: "
		          << SDL_GetError() << "\n";
		std::exit(1);
	}

	bool recording = false;
	// Auto-generated identifier (currently, unix epoch in seconds)
	// that will differ from id of any existing recordings,
	// so that frames can be written to image files
	// without overwriting any files.
	std::time_t recording_id;
	// Current frame number while recording. Frame 0 is first frame.
	int recording_frame_num = 0;

	while (!quit) {
		new_time = SDL_GetTicks();
		delta = new_time - old_time;
		ddelta = (double)delta;
		old_time = new_time;

		text_surface_rerender_timer_ms += delta;

		// Converting spherical coordinates to a vector
		// r = 1 so not shown and no need to normalize the vector
		glm::dvec3 look(
			sin(vang) * cos(hang),
			sin(vang) * sin(hang),
			cos(vang)
		);

		double up_vang = vang - (M_PI / 2.0);
		glm::dvec3 up(
			sin(up_vang) * cos(hang),
			sin(up_vang) * sin(hang),
			cos(up_vang)
		);

		glm::dvec3 forward(
			cos(hang),
			sin(hang),
			0.0
		);

		double right_hang = hang - (M_PI / 2.0);
		glm::dvec3 right(
			cos(right_hang),
			sin(right_hang),
			0.0
		);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
			}
			else if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					// Maybe different call if high DPI allowed
					//  when window created
					SDL_GetWindowSize(window, &screen_width, &screen_height);

					delete[] framebuf;
					framebuf = new Uint8[screen_width * screen_height * 4];

					SDL_DestroyTexture(tex);
					tex = SDL_CreateTexture(
						renderer,
						SDL_PIXELFORMAT_ABGR8888,
						SDL_TEXTUREACCESS_STREAMING,
						screen_width, screen_height);

					if (tex == NULL) {
						std::cerr << "Failed to recreate texture: "
						          << SDL_GetError() << "\n";
						std::exit(1);
					}
				}
				else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
					if (SDL_SetRelativeMouseMode(SDL_TRUE)) {
						std::cerr
							<< "FOCUS_GAINED SDL_SetRelativeMouseMode failed: "
							<< SDL_GetError() << "\n";
					}
				}
				else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
					if (SDL_SetRelativeMouseMode(SDL_FALSE)) {
						std::cerr
							<< "FOCUS_LOST SDL_SetRelativeMouseMode failed: "
							<< SDL_GetError() << "\n";
					}
				}
			}
			else if (event.type == SDL_MOUSEMOTION) {
				hang -= mouse_sens * 0.00025 * event.motion.xrel * ddelta;
				vang += mouse_sens * 0.00025 * event.motion.yrel * ddelta;

				if (vang < 0.0) {
					vang = 0.0;
				}
				else if (vang > M_PI) {
					vang = M_PI;
				}
			}
			else if (event.type == SDL_MOUSEWHEEL) {
				if (image_plane == IMAGEPLANE_PERSPECTIVE ||
					image_plane == IMAGEPLANE_SPHERICAL)
				{
					const double new_hfov =
						hfov - (scroll_sens * 0.03 * event.wheel.y);

					if (new_hfov > 0.0 && new_hfov < M_PI) {
						hfov = new_hfov;
					}
				}
				else if (image_plane == IMAGEPLANE_ORTHOGRAPHIC) {
					const double new_ortho_width =
						ortho_width - (scroll_sens * 0.0075 * event.wheel.y);

					if (new_ortho_width > 0.0) {
						ortho_width = new_ortho_width;
					}
				}
			}
			else if (event.type == SDL_TEXTINPUT && console_active) {
				console_buf.append(event.text.text);
			}
			else if (event.type == SDL_KEYUP) {
				const SDL_Keymod mod_state = SDL_GetModState();

				switch (event.key.keysym.sym) {
				case SDLK_q:
				{
					if (mod_state & KMOD_CTRL) {
						quit = true;
					}

					break;
				}
				case SDLK_BACKQUOTE:
				{
					console_active = !console_active;

					if (console_active) {
						console_buf.assign(" ");
					}

					break;
				}
				case SDLK_BACKSPACE:
				{
					// Do not permit deleting the leading space
					if (console_active && console_buf.length() > 1) {
						console_buf.erase(console_buf.length() - 1, 1);
					}

					break;
				}
				case SDLK_RETURN:
				{
					if (console_active) {
						console_active = false;
						std::istringstream iss(console_buf);
						ConsumeConfigStream(iss);
						console_buf.assign(" ");
					}

					break;
				}
				case SDLK_F1:
				{
					show_fps = !show_fps;
					break;
				}
				case SDLK_F11:
					// Ignore F11 press
					//  if any of Alt, Shift, Ctrl, etc. are down
					if (mod_state != KMOD_NONE) {
						break;
					}

					fullscreen = !fullscreen;

					if (fullscreen) {
						SDL_SetWindowFullscreen(
							window, SDL_WINDOW_FULLSCREEN_DESKTOP);
					}
					else {
						SDL_SetWindowFullscreen(window, 0);
					}

					break;
				case SDLK_F12:
				{
					if (mod_state != KMOD_NONE) {
						break;
					}

					std::time_t seconds = std::time(NULL);

					if (seconds == (std::time_t)(-1)) {
						std::cerr
							<< "Failed to get time for screenshot. "
							<< "Screenshot NOT saved.\n";
					}
					else {
						std::stringstream ss;
						ss << "screenshots/hmap_" << seconds << ".png";
						std::string path = ss.str();

						SavePNG(framebuf, path);
					}

					break;
				}
				case SDLK_1:
					if (!console_active) {
						image_plane = IMAGEPLANE_PERSPECTIVE;
					}

					break;
				case SDLK_2:
					if (!console_active) {
						image_plane = IMAGEPLANE_SPHERICAL;
					}

					break;
				case SDLK_3:
					if (!console_active) {
						image_plane = IMAGEPLANE_ORTHOGRAPHIC;
					}

					break;
				case SDLK_r:
				{
					const bool ctrl_and_shift = (mod_state & KMOD_CTRL) &&
					                            (mod_state & KMOD_SHIFT);

					if (!console_active && ctrl_and_shift) {
						if (!recording) {
							recording = true;
							recording_frame_num = 0;

							recording_id = std::time(NULL);

							// move_speed = 0.0;
							// mouse_sens = 0.0;
							// scroll_sens = 0.0;
							// cycle_period = 1;

							if (recording_id == (std::time_t)(-1)) {
								std::cerr
									<< "Failed to get time for recording. "
									<< "Recording NOT started.\n";

								recording = false;
							}
						}
						else {
							recording = !recording;
						}
					}

					break;
				}
				default:
					break;
				}
			}
		}

		// If you do not intend to manually move the camera while recording,
		// before starting the recording, consider setting in the console:
		//     move 0
		//     mouse_sens 0
		//     scroll_sens 0
		//
		// Also consider `cycle 1` so that frames are complete images.
		//
		// For a programmatic animation, alter this block and recompile.
		if (recording)
		{
			// const double t =
			// 	((double)recording_frame_num) / recording_frame_count;
			//
			// // Change some global parameters.
			//
			// // We need to update the heightmap if we have changed any
			// // global parameters that have an effect on it.
			// UpdateHeightmap();
		}

		const Uint8 *const kb_state = SDL_GetKeyboardState(NULL);
		const SDL_Keymod mod_state = SDL_GetModState();

		if (!console_active && mod_state == KMOD_NONE) {
			if (kb_state[SDL_SCANCODE_W]) {
				cam_pos += ddelta * move_speed * forward;
			}
			if (kb_state[SDL_SCANCODE_S]) {
				cam_pos -= ddelta * move_speed * forward;
			}
			if (kb_state[SDL_SCANCODE_D]) {
				cam_pos += ddelta * move_speed * right;
			}
			if (kb_state[SDL_SCANCODE_A]) {
				cam_pos -= ddelta * move_speed * right;
			}
			if (kb_state[SDL_SCANCODE_SPACE]) {
				cam_pos.z += ddelta * move_speed;
			}
			if (kb_state[SDL_SCANCODE_Q]) {
				cam_pos.z -= ddelta * move_speed;
			}
		}

		ImagePlane *ip;

		if (image_plane == IMAGEPLANE_PERSPECTIVE) {
			ip = new Perspective(cam_pos, look, up, hfov,
				(double)screen_width / screen_height);
		}
		else if (image_plane == IMAGEPLANE_SPHERICAL) {
			ip = new Spherical(cam_pos, hang, vang, hfov,
				(double)screen_width / screen_height);
		}
		else {
			ip = new Orthographic(cam_pos, look, up, ortho_width,
				screen_width, screen_height);
		}

		// Upper left bottom corner of the heightmap
		glm::dvec3 hmap_c0(0.0, 0.0, min_height);
		// Lower right top corner of the heightmap
		glm::dvec3 hmap_c1(
			hmap_c0.x + heightmap_width * grid_width,
			hmap_c0.y - heightmap_height * grid_width,
			max_height
		);

		cycle = (cycle + 1) % cycle_period;

		#pragma omp parallel for
		for (int p = cycle; p < screen_width * screen_height;
		     p += cycle_period)
		{
			int w = p % screen_width;
			int h = p / screen_width;

			struct Ray ray = ip->GetRay(
				(double)w / (screen_width - 1),
				(double)h / (screen_height - 1)
			);

			glm::dvec3 int_point;
			bool hit = intersection(&int_point, ray, hmap_c0, hmap_c1);

			// Did the ray hit the actual heightmap and
			//  not just the bounding box?
			bool real_hit = false;

			if (hit) {
				int_point += grid_width * 0.01 * ray.dir;

				while (true) {
					int gridx =
						(int)( (int_point.x - hmap_c0.x) / grid_width);
					int gridy =
						(int)(-(int_point.y - hmap_c0.y) / grid_width);

					if (gridx < 0 || gridy < 0
						|| gridx >= heightmap_width
						|| gridy >= heightmap_height)
					{
						break;
					}

					double heightmap_z =
						heightmap_buf[gridx + gridy * heightmap_width];

					if (int_point.z < heightmap_z + hmap_c0.z) {
						// Draw
						int red_index = (gridx + gridy * colormap_width) * 4;

						if (colormap_buf[red_index + 3] == 0) {
							SetPixel(framebuf, w, h,
								bg_r, bg_g, bg_b,
								255);
						}
						else {
							SetPixel(framebuf, w, h,
								colormap_buf[red_index + 0],
								colormap_buf[red_index + 1],
								colormap_buf[red_index + 2],
								255);
						}

						real_hit = true;
						break;
					}

					int_point += step_dist * ray.dir;
				}
			}

			if (!real_hit) {
				// Sky-like effect
				if (ray.dir.z > 0.0) {
					const double r_ = 220.0 * std::pow(ray.dir.z, 2) + bg_r;
					const double g_ = 240.0 * std::pow(ray.dir.z, 2) + bg_g;
					const double b_ = 255.0 * ray.dir.z              + bg_b;

					SetPixel(framebuf, w, h,
						(Uint8)std::floor(Clamp<double>(r_, 0.0, 255.0)),
						(Uint8)std::floor(Clamp<double>(g_, 0.0, 255.0)),
						(Uint8)std::floor(Clamp<double>(b_, 0.0, 255.0)),
						255);
				}
				else {
					SetPixel(framebuf, w, h, bg_r, bg_g, bg_b, 255);
				}
			}
		}

		if (text_surface_rerender_timer_ms >= text_surface_rerender_period_ms)
		{
			text_surface_rerender_timer_ms = 0;

			SDL_Color fg = {255, 255, 255, 255};
			SDL_Color bg = {0, 0, 0, 255};

			const double fps = 1000.0 / ddelta;

			std::stringstream ss;
			ss << "FPS: " << std::fixed << std::setprecision(1) << fps;

			SDL_FreeSurface(fps_surface);
			fps_surface = TTF_RenderUTF8_Shaded(font, ss.str().c_str(), fg, bg);

			bg = (SDL_Color){50, 100, 250, 255};

			SDL_FreeSurface(console_surface);
			console_surface = TTF_RenderUTF8_Shaded(
				font, console_buf.c_str(), fg, bg);
		}

		SDL_UpdateTexture(tex, NULL, framebuf, screen_width * 4);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, tex, NULL, NULL);

		// fps_surface and console_surface will not appear in screenshots
		//  and recordings because they are sent 'straight to the renderer'
		//  rather than being put into `framebuf`

		if (show_fps && fps_surface != NULL) {
			SDL_Texture *const ftex =
				SDL_CreateTextureFromSurface(renderer, fps_surface);

			if (ftex == NULL) {
				std::cerr
					<< "Failed to create texture from fps_surface: "
					<< SDL_GetError() << "\n";
				break;
			}

			SDL_Rect dst_rect = {5, 2, fps_surface->w, fps_surface->h};
			SDL_RenderCopy(renderer, ftex, NULL, &dst_rect);

			SDL_DestroyTexture(ftex);
		}

		if (console_active && console_surface != NULL) {
			SDL_Texture *const ftex =
				SDL_CreateTextureFromSurface(renderer, console_surface);

			if (ftex == NULL) {
				std::cerr
					<< "Failed to create texture from console_surface: "
					<< SDL_GetError() << "\n";
				break;
			}

			SDL_Rect dst_rect = {
				0, screen_height - console_surface->h - 10,
				console_surface->w, console_surface->h
			};
			SDL_RenderCopy(renderer, ftex, NULL, &dst_rect);

			SDL_DestroyTexture(ftex);
		}

		SDL_RenderPresent(renderer);

		delete ip;

		if (recording) {
			std::stringstream ss;
			ss << "screenshots/hmap_" << recording_id << "_"
			   << recording_frame_num << ".png";

			SavePNG(framebuf, ss.str());

			recording_frame_num += 1;

			if (recording_frame_num == recording_frame_count) {
				recording = false;
				std::cout << "Done recording.\n";
			}
		}
	} // while (!quit)

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(tex);
	SDL_FreeSurface(fps_surface);
	SDL_FreeSurface(console_surface);

	TTF_CloseFont(font);

	stbi_image_free((void*)base_heightmap_buf);
	delete[] heightmap_buf;
	stbi_image_free((void*)colormap_buf);

	delete[] framebuf;

	return 0;
}
