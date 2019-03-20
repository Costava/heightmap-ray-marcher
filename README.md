# heightmap-ray-marcher

A ray marcher for heightmap images or any images.  
Move the camera with WASD and the mouse.

Feel free to ask a question by opening an issue.

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
