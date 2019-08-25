@echo off

:: Using **MinGW Distro** from https://nuwen.net/mingw.html (StephanTLavavej).
:: It contains SDL2, but not SDL2_gfx.
:: 
:: SDL2_gfx was build by hands using MinGW Distro and
:: StephanTLavavej build scripts: https://github.com/StephanTLavavej/mingw-distro.
:: Check README.md (https://github.com/StephanTLavavej/mingw-distro/blob/master/README.md)
:: to see how to configure MSYS2.
:: 
:: Assuming you have C:\Temp\gcc\msys64 *and* installed MinGW Distro to C:\Programs\mingw_gcc_7.3.0.
::
:: * copy StephanTLavavej build scripts to C:\Temp\gcc\sources
::   (either from github or from installed distro: C:\Programs\mingw_gcc_7.3.0\scripts-15.4).
::   You should have C:\Temp\gcc\sources\0_append_distro_path.sh among other files
::   as result;
:: * copy SDL2-2.0.7.tar, SDL2_gfx-1.0.4.tar (need to download them) files
::   together with mingw_sdl_only_with_gfx_unpack.sh and mingw_sdl_gfx_compile.sh files
::   to C:\Temp\gcc\sources;
:: * run `msys2_shell.cmd`;
:: * execute `export PATH=${PATH}:/C/Programs/mingw_gcc_7.3.0/bin`;
:: * execute `./mingw_sdl_only_with_gfx_unpack.sh`;
:: * execute `./mingw_sdl_gfx_compile.sh`;
:: 
:: Copy SDL2_gfxPrimitives.h, SDL2_imageFilter.h, SDL2_rotozoom.h
:: from C:\Temp\gcc\dest\include\SDL2
:: to C:\Programs\mingw_gcc_7.3.0\include\SDL2;
:: copy C:\Temp\gcc\dest\lib\libSDL2_gfx.a
:: to C:\Programs\mingw_gcc_7.3.0\lib

set gcc_includes=C:\Programs\mingw_gcc_7.3.0\include
set gcc_libs=C:\Programs\mingw_gcc_7.3.0\lib
set gcc=C:\Programs\mingw_gcc_7.3.0\bin\g++.exe

%gcc% -std=c++17 -O2 snake.cpp ^
	-o snake_gcc.exe ^
	-I%gcc_includes% -L%gcc_libs% ^
	-lSDL2 -lSDL2_gfx ^
	-lmingw32 -mwindows -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc
:: this last line is SDL2 dependencies/linker and
:: comes from `./sdl2-config --libs`

