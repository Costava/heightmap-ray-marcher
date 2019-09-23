@echo off
setlocal

call BuildVars.cmd %1

rem "use the multithread-specific and DLL-specific version of the run-time library"
rem via https://docs.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library
set runtime=/MD

rem https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model
rem Exception Handling Model
set ehm=/EHsc

cl %in% %ehm% %runtime% /I %sdl_include% /link /out:%out% /LIBPATH:%sdl_lib_path% SDL2.lib SDL2main.lib /SUBSYSTEM:%subsystem%
