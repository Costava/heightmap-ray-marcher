@echo off
setlocal

call BuildVars.cmd %1

rem https://clang.llvm.org/docs/ClangCommandLineReference.html
rem https://stackoverflow.com/questions/39871384/error-when-linking-sdl2-using-clang-on-windows-lnk1561-entry-point-must-be-def

clang %in% --output %out% --include-directory %sdl_include% --library-directory %sdl_lib_path% -lSDL2main -lSDL2 --for-linker /subsystem:%subsystem%
