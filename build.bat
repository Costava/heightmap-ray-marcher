@ECHO OFF
SETLOCAL

:: Download the Visual C++ development libraries from
::  https://www.libsdl.org/download-2.0.php
:: And create symbolic link `SDL2-2.0.9` (in the same directory as this file)
::  to the location of the downloaded SDL2-2.0.9 directory
SET sdl_include=SDL2-2.0.9\include
SET sdl_libpath=SDL2-2.0.9\lib\x64

:: "use the multithread-specific and DLL-specific version of the run-time library"
:: via https://docs.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library
SET runtime=/MD

:: https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model
:: Exception Handling Model
SET ehm=/EHsc

SET in=src\main.cpp src\Perspective.cpp src\Spherical.cpp src\Orthographic.cpp src\AABB.cpp
SET out=hmap.exe

SET subsystem=WINDOWS
IF "%1"=="dev" (
	SET subsystem=CONSOLE
)

CL %in% %ehm% %runtime% /I %sdl_include% /link /out:%out% /LIBPATH:%sdl_libpath% SDL2.lib SDL2main.lib /SUBSYSTEM:%subsystem%
