set sdl_include=src\SDL\include
set sdl_lib_path=src\SDL\lib\x64

set in=src\main.cpp src\Perspective.cpp src\Spherical.cpp src\Orthographic.cpp src\AABB.cpp
set out=hmap.exe

set subsystem=CONSOLE
IF "%1"=="prod" (
	set subsystem=WINDOWS
)
