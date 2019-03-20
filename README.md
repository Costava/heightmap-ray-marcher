# heightmap-ray-marcher

A ray marcher for heightmap images or any images.  
Move the camera with WASD and the mouse. Hold space to raise the camera and hold shift to lower it.  
Switch the projection mode with the number keys:
1. Perspective
2. Spherical
3. Orthographic

Press escape to exit.  
Press F11 to toggle fullscreen.  
You can freely resize the window.

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

![Face in leaves (close)](https://i.imgur.com/MQwqKdL.png)
![Face in leaves (overview)](https://i.imgur.com/voGVGgZ.png)  
[Original image](https://unsplash.com/photos/svnH68VDN4Q)

## Configuration file
The program is launched from the command line with a configuration file argument: `hmap.exe path/to/config.txt`.  
Every line in the config starts with an identifier followed by the appropriate arguments for that identifier separated by spaces.  
All config file options are optional except `heightmap` and `colormap`.  
Options can be specified in any order, but each should have its own line.
Currently, input to many options is not checked for validity.  
See `sample_config.txt` for an example.

### Options
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

`heightmap path/to/img.jpg`: A path to an image. The image does NOT have to be greyscale. The luminance of the image's pixels determine the heights. The image can be any format supported by `stb_image.h`: JPEG, PNG, TGA, BMP, PSD, GIF, HDR, PIC, PNM. See `src/stb_image.h` for details and exceptions. The image must have the same resolution as the image for `colormap`. This option must be specified.

`colormap path/to/img.jpg`: Similar to `heightmap` but for determining colors. The easiest and most often best choice will be the same image as for `heightmap`. The image must have the same resolution as the image for `heightmap`. This option must be specified.

## Build

This covers building on Windows 64-bit and 32-bit. On Linux and macOS, use your favorite C/C++ compiler and the appropriate SDL2 distribution. This project uses the current stable SDL version `2.0.9`.

### Download
- Download and install [Build Tools for Visual Studio 2017](https://www.visualstudio.com/downloads) to get the `CL` C/C++ compiler and `x64 Native Tools Command Prompt`. Use `x86 Native Tools Command Prompt` for 32-bit Windows. More information on these command prompts  [here](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line).
- Download the Visual C++ [SDL2 development libraries](https://www.libsdl.org/download-2.0.php).
- Download [OpenGL Mathematics 0.9.9.3](https://glm.g-truc.net/0.9.9/index.html)

### Symbolic links
- Create a symbolic link named `SDL2-2.0.9` targeting the location of the downloaded `SDL2-2.0.9` folder. Likely use `mklink /D SDL2-2.0.9 C:\path\to\SDL2-2.0.9`. The `/D` argument is for directory symbolic link as opposed to the default file symbolic link.
- Create link `glm` inside `src` targeting your `glm` folder

### How to
`build.bat` builds the executable for 64-bit Windows, but the project can be built for 32-bit and 64-bit Windows/Linux/macOS with an appropriate compiler and SDL2 distribution.

Run `build.bat` in the command prompt from above to build. Run `build.bat dev` to get a console when the program runs.

Switch `x64` to `x86` for `sdl_libpath` in `build.bat` for 32-bit.

Run the resulting executable `hmap.exe` with a path argument to a config file: `hmap.exe path/to/config.txt`
