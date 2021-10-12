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
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include "stb_image.h"
#include "stb_image_write.h"

#include "AABB.hpp"
#include "ImagePlane.hpp"
#include "Perspective.hpp"
#include "Spherical.hpp"
#include "Orthographic.hpp"

// Window content dimensions (pixels)
int screen_width = 512;
int screen_height = 512;

// Horizontal field of view (radians)
double hfov = M_PI / 2.0;

// Normalize heights in heightmap between these bounds
double min_height = 0.0;
double max_height = 10.0;

// Array of heights
double *heightmap_buf = NULL;
int heightmap_width;
int heightmap_height;

// Array of RGB unsigned char values
unsigned char *colormap_buf = NULL;
int colormap_width;
int colormap_height;

// Width and height of grid cells of image heightmap
// i.e. how far apart pixels from the image are in world space when rendered
double grid_width = 0.003;

// How far to step at a time when raymarching
double step_dist = 10.0 * grid_width;

// Num bits to use for a render cycle
// Must be even
// If num_bits == 4, then a complete image will be drawn in 2^4 = 16 frames
unsigned int num_bits = 6;
unsigned int shift_amt;
unsigned int full_mask;
unsigned int half_mask;
unsigned int incr;

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

// Multiplier for how many radians to turn
// For both horizontal and vertical mouse sensitivity
double mouse_sens = 0.00003;

// Render only part of the screen in a single frame
// Keep track of which part to render next
int render_cycle = 0;

bool fullscreen = false;

// How far apart the orthographic rays are
double ortho_width = 10.0 * grid_width;

#define IMAGEPLANE_PERSPECTIVE  1
#define IMAGEPLANE_SPHERICAL    2
#define IMAGEPLANE_ORTHOGRAPHIC 3
int image_plane = IMAGEPLANE_PERSPECTIVE;

// Background color
Uint8 bg_r = 0;
Uint8 bg_g = 0;
Uint8 bg_b = 0;

double DegreesToRads(const double degrees) {
	return (degrees / 180.0) * M_PI;
}

double RadsToDegrees(const double rads) {
	return (rads / M_PI) * 180.0;
}

// This makes assumptions about the format of the surface
// Could be improved to be more robust (but likely less performant)
void SetPixel(
	SDL_Surface *const surface,
	const int x,
	const int y,
	const Uint8 r,
	const Uint8 g,
	const Uint8 b)
{
	// Assume BytesPerPixel==4
	Uint32 *const pixels = (Uint32 *) surface->pixels;

	const Uint32 color = SDL_MapRGB(surface->format, r, g, b);

	// Assume pitch has no dead space
	pixels[x + (y * surface->w)] = color;
}

// num_bits should be multiple of 2 in range [min_cycle_bits, max_cycle_bits]
unsigned int min_cycle_bits;
unsigned int max_cycle_bits;

// Update variables related to the render cycle based on using num_bits
void UpdateCycleVars(
	const unsigned int num_bits,
	unsigned int *const shift_amt,
	unsigned int *const full_mask,
	unsigned int *const half_mask,
	unsigned int *const incr)
{
	const unsigned int NUM_UINT_BITS = 8 * sizeof(unsigned int);

	*shift_amt = num_bits / 2;
	*full_mask = (UINT_MAX << (NUM_UINT_BITS - num_bits))
	             >> (NUM_UINT_BITS - num_bits);
	*half_mask = *full_mask >> (num_bits / 2);
	*incr = *half_mask + 1;
}

// Read stream until end and update config values
void ConsumeConfigStream(std::istream &input) {
	std::string next;
	while (input >> next) {
		if (next == "resolution") {
			int width;
			int height;

			input >> width >> height;

			std::cout << "resolution: " << width << " by " << height << "\n";

			screen_width = width;
			screen_height = height;
		}
		else if (next == "hfov") {
			double hf;
			input >> hf;

			double new_hfov = DegreesToRads(hf);

			std::cout << "hfov: " << hf << " degrees"
			          << "\n";

			hfov = new_hfov;
		}
		else if (next == "hang") {
			double deg;
			input >> deg;

			hang = DegreesToRads(deg);
		}
		else if (next == "vang") {
			double deg;
			input >> deg;

			vang = DegreesToRads(deg);
		}
		else if (next == "pos") {
			double x, y, z;
			input >> x >> y >> z;

			cam_pos.x = x;
			cam_pos.y = y;
			cam_pos.z = z;
		}
		else if (next == "print_pos") {
			std::cout
				<< "pos: " << glm::to_string(cam_pos) << "\n"
				<< "hang vang: "
				<< RadsToDegrees(hang) << " " << RadsToDegrees(vang) << "\n"
				<< std::flush;
		}
		else if (next == "min_height") {
			input >> min_height;

			std::cout << "min_height: " << min_height << "\n";
		}
		else if (next == "max_height") {
			input >> max_height;

			std::cout << "max_height: " << max_height << "\n";
		}
		else if (next == "grid_width") {
			input >> grid_width;

			std::cout << "grid_width: " << grid_width << "\n";
		}
		else if (next == "ortho_width") {
			input >> ortho_width;

			std::cout << "ortho_width: " << ortho_width << "\n";
		}
		else if (next == "step_dist") {
			input >> step_dist;

			std::cout << "step_dist: " << step_dist << "\n";
		}
		else if (next == "bg_color") {
			int r, g, b;

			input >> r >> g >> b;

			bg_r = (Uint8)r;
			bg_g = (Uint8)g;
			bg_b = (Uint8)b;

			std::cout << "bg_color:" << "\n";
			std::cout << "\tr: " << (int)bg_r << "\n";
			std::cout << "\tg: " << (int)bg_g << "\n";
			std::cout << "\tb: " << (int)bg_b << "\n";
		}
		else if (next == "cycle_bits") {
			int cyc;

			input >> cyc;

			if (cyc < 2) {
				std::cout
					<< "Ignoring cycle bits " << cyc << " because is < 2\n";
			}
			else if (cyc % 2 != 0) {
				std::cout
					<< "Ignoring cycle bits " << cyc
					<< " because is not a multiple of 2\n";
			}
			else if ((unsigned int)cyc < min_cycle_bits ||
			         (unsigned int)cyc > max_cycle_bits)
			{
				std::cout
					<< "Ignoring cycle bits " << cyc
					<< " because not in range [" << min_cycle_bits << ", "
					<< max_cycle_bits << "]\n";
			}
			else {
				std::cout << "cycle bits: " << cyc << "\n";

				num_bits = cyc;

				UpdateCycleVars(
					num_bits, &shift_amt, &full_mask, &half_mask, &incr);
			}
		}
		else if (next == "mouse_sens") {
			input >> mouse_sens;

			std::cout << "mouse_sens: " << mouse_sens << "\n";
		}
		else if (next == "heightmap") {
			std::string path;
			input >> path;

			int n;

			// Load image as greyscale
			const unsigned char *const data = stbi_load(
				path.c_str(), &heightmap_width, &heightmap_height, &n, 1);

			if (data == NULL) {
				std::cerr
					<< "Failed to load image for heightmap from "
					<< path << "\n";
				std::exit(1);
			}

			const int length = heightmap_width * heightmap_height;
			delete heightmap_buf;
			heightmap_buf = new double[length];

			for (int i = 0; i < length; i += 1) {
				heightmap_buf[i] = ((double)data[i] / 255.0)
					* (max_height - min_height) + min_height;
			}

			stbi_image_free((void*)data);

			std::cout << "heightmap: " << path << "\n";
		}
		else if (next == "colormap") {
			std::string path;
			input >> path;

			int n;

			stbi_image_free(colormap_buf);
			colormap_buf = stbi_load(
				path.c_str(), &colormap_width, &colormap_height, &n, 3);

			if (colormap_buf == NULL) {
				std::cerr
					<< "Failed to load image for colormap from "
					<< path << "\n";
				std::exit(1);
			}

			std::cout << "colormap: " << path << "\n";
		}
		else {
			std::cout << "WARNING: Unknown identifier: " << next << "\n";
		}
	}
}

int main(int argc, char *argv[]) {
	const unsigned int NUM_INT_BITS = 8 * sizeof(int);
	const unsigned int NUM_UINT_BITS = 8 * sizeof(unsigned int);

	if (NUM_INT_BITS != NUM_UINT_BITS) {
		std::cerr
			<< "Unknown int and unsigned int implementations" << "\n"
			<< "NUM_INT_BITS: " << NUM_INT_BITS << "\n"
			<< "NUM_UINT_BITS: " << NUM_UINT_BITS << "\n";

		std::exit(1);
	}

	min_cycle_bits = 2;
	max_cycle_bits = NUM_UINT_BITS;

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

	// Some validation

	if (heightmap_buf == NULL) {
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

	SDL_Window *const window = SDL_CreateWindow(
		"Heightmap Ray Marcher",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_width, screen_height,
		SDL_WINDOW_RESIZABLE);

	if (window == NULL) {
		std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
		std::exit(1);
	}

	// "This surface will be freed when the window is destroyed.
	//  Do not free this surface."
	// https://wiki.libsdl.org/SDL_GetWindowSurface
	SDL_Surface *surface = SDL_GetWindowSurface(window);

	// Initialize window to all background color
	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, bg_r, bg_g, bg_b));
	SDL_UpdateWindowSurface(window);

	std::srand((unsigned)std::time(NULL));

	UpdateCycleVars(num_bits, &shift_amt, &full_mask, &half_mask, &incr);

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
					surface = SDL_GetWindowSurface(window);

					// Maybe different call if high DPI allowed
					//  when window created
					SDL_GetWindowSize(window, &screen_width, &screen_height);

					// Show immediately that the screen has been
					//  cleared by the resize
					SDL_UpdateWindowSurface(window);
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
				hang -= mouse_sens * event.motion.xrel * ddelta;
				vang += mouse_sens * event.motion.yrel * ddelta;

				if (vang < 0.0) {
					vang = 0.0;
				}
				else if (vang > M_PI) {
					vang = M_PI;
				}
			}
			else if (event.type == SDL_TEXTINPUT && console_active) {
				console_buf.append(event.text.text);
			}
			else if (event.type == SDL_KEYUP) {
				const SDL_Keymod mod_state = SDL_GetModState();

				switch (event.key.keysym.sym) {
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
				case SDLK_ESCAPE:
					quit = true;
					break;
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

					if (surface->format->BytesPerPixel != 4) {
						// Who owns the value
						//  returned by SDL_GetPixelFormatName?
						// I will assume I do not need to free it.
						// https://wiki.libsdl.org/SDL_GetPixelFormatName

						std::cerr
							<< "Unhandled PixelFormat "
							<< SDL_GetPixelFormatName(surface->format->format)
							<< " has BytesPerPixel="
							<< surface->format->BytesPerPixel
							<< ". Screenshot NOT saved.\n";

						break;
					}

					std::time_t seconds = std::time(NULL);

					if (seconds == (std::time_t)(-1)) {
						std::cerr
							<< "Failed to get time for screenshot. "
							<< "Screenshot NOT saved.\n";

						break;
					}

					std::stringstream ss;
					ss << "screenshots/hmap_" << seconds << ".png";
					std::string path = ss.str();

					Uint8 *const data = new Uint8[surface->w * surface->h * 3];

					for (int i = 0; i < surface->w * surface->h; i += 1) {
						const Uint32 pixel = ((Uint32*)surface->pixels)[i];

						SDL_GetRGB(pixel, surface->format,
							&data[i * 3 + 0],
							&data[i * 3 + 1],
							&data[i * 3 + 2]);
					}

					const int code = stbi_write_png(path.c_str(),
						surface->w, surface->h, 3,
						data, surface->w * 3);

					if (code == 0) {
						std::cerr
							<< "Failed to write screenshot to " << path << "\n";
					}
					else {
						std::cout << "Saved screenshot at " << path << "\n";
					}

					delete data;
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
				default:
					break;
				}
			}
		}

		const Uint8 *const kb_state = SDL_GetKeyboardState(NULL);

		if (!console_active) {
			const double move_mult = 0.01;

			if (kb_state[SDL_SCANCODE_W]) {
				cam_pos += ddelta * move_mult * forward;
			}
			if (kb_state[SDL_SCANCODE_S]) {
				cam_pos -= ddelta * move_mult * forward;
			}
			if (kb_state[SDL_SCANCODE_D]) {
				cam_pos += ddelta * move_mult * right;
			}
			if (kb_state[SDL_SCANCODE_A]) {
				cam_pos -= ddelta * move_mult * right;
			}
			if (kb_state[SDL_SCANCODE_SPACE]) {
				cam_pos.z += ddelta * move_mult;
			}
			if (kb_state[SDL_SCANCODE_LSHIFT]) {
				cam_pos.z -= ddelta * move_mult;
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

		// // Clear window to black
		// SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));

		// Upper left bottom corner of the heightmap
		glm::dvec3 hmap_c0(0.0, 0.0, min_height);
		// Lower right top corner of the heightmap
		glm::dvec3 hmap_c1(
			hmap_c0.x + heightmap_width * grid_width,
			hmap_c0.y - heightmap_height * grid_width,
			max_height
		);

		// // 4 bit cycle
		// // Look at least significant bits
		// int initial_h = render_cycle & 0b11;
		// int initial_w = (render_cycle >> 2) & 0b11;
		// render_cycle = (render_cycle + 1) & 0xf;// Increment render cycle

		// // 8 bit cycle
		// // Look at least significant bits
		// int initial_h = render_cycle & 0b1111;
		// int initial_w = (render_cycle >> 4) & 0b1111;
		// render_cycle = (render_cycle + 1) & 0xff;// Increment render cycle

		int initial_h = render_cycle & half_mask;
		int initial_w = (render_cycle >> shift_amt) & half_mask;
		render_cycle = (render_cycle + 1) & full_mask;

		for (int h = initial_h; h < screen_height; h += incr) {
			for (int w = initial_w; w < screen_width; w += incr) {
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
							int red_index =
								(gridx + gridy * colormap_width) * 3;

							SetPixel(surface, w, h,
								colormap_buf[red_index],
								colormap_buf[red_index + 1],
								colormap_buf[red_index + 2]);

							real_hit = true;
							break;
						}

						int_point += step_dist * ray.dir;
					}
				}

				if (!real_hit) {
					SetPixel(surface, w, h, bg_r, bg_g, bg_b);
				}

				// // Debug heightmap box render
				// if (hit) {
				// 	SetPixel(surface, w, h,
				// 		(int)(int_point.x * 255) % 255,
				// 		(int)(int_point.y * 255) % 255,
				// 		(int)(int_point.z * 255) % 255
				// 	);
				// }

				// // Debug plane render
				// if (ray.dir.z < 0.0) {
				// 	SetPixel(surface, w, h,
				// 		(int)(ray.dir.x * 255) % 255,
				// 		(int)(ray.dir.y * 255) % 255,
				// 		128);
				// }
			}
		}

		if (text_surface_rerender_timer_ms >= text_surface_rerender_period_ms) {
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

		if (show_fps) {
			if (fps_surface == NULL) {
				break;
			}

			SDL_Rect dst_rect = {5, 2, fps_surface->w, fps_surface->h};

			SDL_BlitSurface(fps_surface, NULL, surface, &dst_rect);
		}

		if (console_active) {
			if (console_surface == NULL) {
				break;
			}

			SDL_Rect dst_rect = {
				0, surface->h - console_surface->h - 10,
				console_surface->w, console_surface->h
			};
			SDL_BlitSurface(console_surface, NULL, surface, &dst_rect);
		}

		SDL_UpdateWindowSurface(window);

		delete ip;
	} // while (!quit)

	SDL_FreeSurface(fps_surface);

	SDL_DestroyWindow(window);

	TTF_CloseFont(font);

	delete heightmap_buf;
	stbi_image_free(colormap_buf);

	return 0;
}