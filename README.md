# heightmap-ray-marcher

A ray marcher for heightmap images (or any images) with a real time freely moving camera.

Move the camera with WASD and the mouse. Hold space to raise the camera and hold shift to lower it.  
Switch the projection mode with the number keys:
1. Perspective
2. Spherical
3. Orthographic

Press escape to exit.  
Press F11 to toggle fullscreen.  
You can freely resize the window.

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

![White tiger](https://i.imgur.com/EP4EnZ9.png)  
[Original image](https://unsplash.com/photos/dGMcpbzcq1I)

![Face (close)](https://i.imgur.com/dxoOZgR.png)  
![Face (overview)](https://i.imgur.com/YJSp7cq.png)  
[Original image](https://unsplash.com/photos/sibVwORYqs0)

![Face in leaves (close)](https://i.imgur.com/MQwqKdL.png)  
![Face in leaves (overview)](https://i.imgur.com/voGVGgZ.png)  
[Original image](https://unsplash.com/photos/svnH68VDN4Q)

## Configuration file
The program is launched from the command line with a configuration file argument: `hmap.exe path/to/config.txt`.  
Every line in the config starts with an identifier followed by the appropriate arguments for that identifier separated by spaces.  
All config file options are optional except `heightmap` and `colormap`.  
Options can be specified in any order, but each option should have its own line.  
Currently, input to many options is not checked for validity.  
See `sample_config.txt` for an example.

### Options
`heightmap path/to/img.jpg`: A path to an image. The image does NOT have to be greyscale. The luminance of the image's pixels determine the heights. The image can be any format supported by `stb_image.h`: JPEG, PNG, TGA, BMP, PSD, GIF, HDR, PIC, PNM. See `src/stb_image.h` for details and exceptions. The image must have the same resolution as the image for `colormap`. **This option must be specified.**

`colormap path/to/img.jpg`: Similar to `heightmap` but for determining colors. The easiest choice will be the same image as for `heightmap`. The specified image must have the same resolution as the image for `heightmap`. **This option must be specified.**

`resolution <int x> <int y>`: The x and y dimensions (in pixels) of the window content.

`hfov <double degrees>`: Set the horizontal field of view (in degrees). You will likely experience issues if this is not in the range (0, 180).

`min_height <double z>`: The minimum world space height in the height map. Values are normalized between the min and max.

`max_height <double z>`: The maximum world space height in the height map.

`grid_width <double val>`: The world space grid square size of the heightmap.

`ortho_width <double val>`: The world space grid spacing of rays when using orthographic projection.

`step_dist <double val>`: How far in world space to step when ray marching.

`bg_color <int red> <int green> <int blue>`: The background color. Values should be in range [0, 255].

`cycle_bits <int num>`: The number of bits to use when keeping track of which part of the screen to render next. A full image will be rendered in 2^num frames. This is to help keep the frame rate more reasonable. The image gets updated in an order similar to a dither pattern. The value should be a multiple of 2 in the range [2, 32]. The upper limit depends on the `sizeof(unsigned int)` which is likely 4 bytes on your machine.

`mouse_sens <double val>`: Multiplier for how many radians to turn the camera when using the mouse. This applies to both horizontal and vertical rotation.

## Build and run on Linux

Tested with SDL 2.0.12 and glm 0.9.9.8  
The makefile does incremental compilation.

1. Clone the repo
2. Install [SDL2](https://www.libsdl.org/) and [OpenGL Mathematics](https://glm.g-truc.net/) (might be named `glm`) through your package manager
3. `make init`
4. `make build`
5. Run with `./hmap path/to/config.txt`

## Build and run on Windows

- Download and install [Build Tools for Visual Studio 2019](https://www.visualstudio.com/downloads) to get the `CL` C/C++ compiler and `x64 Native Tools Command Prompt`.
- - Use `x86 Native Tools Command Prompt` for 32-bit Windows.
- - More information on these command prompts [here](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line).
- Get a copy of SDL 2.0.9. Source and builds available [here](https://www.libsdl.org/download-2.0.php) (newer versions likely work also).
- Download [OpenGL Mathematics 0.9.9.3](https://glm.g-truc.net/0.9.9/index.html) (newer versions likely work also).

### Symbolic links
- In the `src` folder, create a symbolic link named `SDL` targeting your copy of SDL.
- - `mklink /D src\SDL C:\path\to\SDL2-2.0.9`
- In the `src` folder, create link `glm` targeting your `glm` folder.
- - `mklink /D src\glm C:\path\to\glm`

### How to
`BuildCl.cmd` and `BuildClang.cmd` build the executable for 64-bit Windows using either `CL` or `Clang` respectively.

Run the scripts in the command prompt from above to build. Pass argument `prod` to the scripts for the `WINDOWS` subsystem (no console/printouts when program runs).

Switch `x64` to `x86` for `sdl_lib_path` in `BuildVars.cmd` for 32-bit.

Run the resulting executable `hmap.exe` with a path argument to a config file: `hmap.exe path/to/config.txt`

## License

GNU General Public License v3. See file `LICENSE`.

## Contributing

Not currently accepting contributions. Feel free to create an issue.
