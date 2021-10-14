# heightmap-ray-marcher

A ray marcher for heightmap images (or any images) with a real time freely moving camera.

Move the camera with WASD and the mouse. Hold space to raise the camera and hold shift to lower it.  
Switch the projection mode with the number keys:

1. Perspective
2. Spherical
3. Orthographic

Also:

- Ctrl+Q to exit.
- Press F1 to toggle showing the frames per second.
- Press F11 to toggle fullscreen.
- Press F12 to save a screenshot in `screenshots` directory.
- Press backtick (left of `1`) to toggle the console for changing configuration at runtime.
- Parsing the console input is the same as the parsing for the config file.
- Press Ctrl+Shift+R to begin recording (saving frames out to image files) or to stop recording early (otherwise recording will stop after `recording_frame_count` number of frames).
- You can freely resize the window.

For each pixel, a ray is cast and intersection is checked with an axis aligned bounding box (AABB) around the heightmap.
If the ray collides with the AABB, then ray marching begins at the collision point.

Feel free to ask a question by opening an issue.

## What it can do

These three images were created with the same camera position and direction, but different projection modes.

Perspective:  
![Man (perspective)](https://i.imgur.com/hOn8tdS.png)

Spherical:  
![Man (spherical)](https://i.imgur.com/QJzDTO5.png)

Orthographic:  
![Man (orthographic)](https://i.imgur.com/cHbKgmL.png)  
[Original image](https://unsplash.com/photos/rpF3p_RrE9g)

---

![White tiger](https://i.imgur.com/EP4EnZ9.png)  
[Original image](https://unsplash.com/photos/dGMcpbzcq1I)

---

![Face (close)](https://i.imgur.com/dxoOZgR.png)  
![Face (overview)](https://i.imgur.com/YJSp7cq.png)  
[Original image](https://unsplash.com/photos/sibVwORYqs0)

---

![Face in leaves (close)](https://i.imgur.com/MQwqKdL.png)  
![Face in leaves (overview)](https://i.imgur.com/voGVGgZ.png)  
[Original image](https://unsplash.com/photos/svnH68VDN4Q)

## Configuration file
- The program is launched from the command line with a configuration file argument: `./hmap path/to/config.txt`
- The config file is a sequence of whitespace-separated values.
- Consider starting each line in the config with an identifier, followed by its appropriate arguments.
- Options can be specified in any order.
- All config file options are optional except `heightmap` and `colormap`
- Input to many options is not validated and handling parsing is not robust.
- Do NOT use file paths that include spaces.

See `sample_config.txt` for an example.

### Options

| Identifier | Parameter(s) | Description |
| ---------- | ------------ | ----------- |
| heightmap | path/to/img.png | A path to an image. The image does NOT have to be greyscale. The luminance of the image's pixels determine the heights. The image can be any format supported by `stb_image.h`: JPEG, PNG, TGA, BMP, PSD, GIF, HDR, PIC, PNM. See `src/stb_image.h` for details and exceptions. The image must have the same resolution as the image for `colormap`. **This option must be specified.** |
| colormap | path/to/img.png | Similar to `heightmap` but for determining colors. The easiest choice will be the same image as for `heightmap`. The specified image must have the same resolution as the image for `heightmap`. **This option must be specified.** |
| resolution | \<int x> \<int y> | The x and y dimensions (in pixels) of the window content. |
| hfov | \<double degrees> | Set the horizontal field of view (in degrees). You will likely experience issues if this is not in the range (0, 180). |
| hang | \<double degrees> | Horizontal angle of camera. 0 is looking in direction of positive x axis. 90 is looking in direction of positive y axis, |
| vang | \<double degrees> | Vertical angle of camera. 0 is looking straight up (with positive z axis). 90 is looking parallel to xy plane. |
| pos | \<double x> \<double y> \<double z> | Position of camera |
| print_pos | NA | Prints to stdout the current position and horizontal and vertical viewing angles of the camera. |
| min_height | \<double z> | The minimum world space height in the height map. Values are normalized between the min and max. |
| max_height | \<double z> | The maximum world space height in the height map. |
| lum | \<double r> \<double g> \<double b> | For each pixel in the given heightmap image with components RGB, the pixel's heightmap value is (rR + gG + bB), clamped to range [0.0, 255.0], then scaled to range [min_height, max_height] |
| grid_width | \<double val> | The world space grid square size of the heightmap. |
| ortho_width | \<double val> | The world space grid spacing of rays when using orthographic projection. |
| step_dist | \<double val> | How far in world space to step when ray marching. |
| bg_color | \<int red> \<int green> \<int blue> | The background color. Values should be in range [0, 255]. |
| cycle | \<int num> | A full image will be rendered across `num` frames. |
| mouse_sens | \<double val> | Horizontal and vertical mouse sensitivity for rotating camera. |
| move | \<double val> | Movement speed multiplier. |
| recording_frame_count | \<int count> | The number of frames to render when recording (saving frames out to image files). |

## Build and run on Linux

1. Clone the repo
2. Install the dependencies e.g. `pamac install sdl2 sdl2_ttf glm`
- - [SDL](https://www.libsdl.org/) (tested with v2.0.16)
- - [SDL_ttf](https://www.libsdl.org/projects/SDL_ttf/) (tested with v2.0.15)
- - [OpenGL Mathematics](https://glm.g-truc.net/) (tested with v0.9.9.8)
3. `make build`
4. Run with `./hmap path/to/config.txt`

Building for other platforms will follow a similar approach.

## License

Files original to this repo are under the BSD 2-Clause License.
See file `LICENSE.txt`.

Files under `vendor` and `fonts` have their own licenses.
See those directories for details.

## Contributing

Not currently accepting contributions. Feel free to create an issue.
