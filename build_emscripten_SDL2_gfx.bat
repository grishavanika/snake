@echo off

:: script to build SDL2 gfx on Windows
:: with no ./configure;
:: 
:: can be easily done in WSL with bash, see:
:: https://github.com/gsathya/SDL-emscripten/wiki/SDL-extensions.
:: 
:: Basically:
:: EMCONFIGURE_JS=1 emconfigure ../configure --disable-mmx

pushd

set em_root=C:\dev\_opensource\emsdk
set SDL2_gfx_src=C:\libs\SDL2_gfx-1.0.4

cd /d %em_root%
call emsdk_env.bat

cd /d %SDL2_gfx_src%
mkdir build & cd build

echo Building .c files...

call emcc -s USE_SDL=2 -c -O2 -o SDL2_framerate.o ../SDL2_framerate.c
call emcc -s USE_SDL=2 -c -O2 -o SDL2_gfxPrimitives.o ../SDL2_gfxPrimitives.c
call emcc -s USE_SDL=2 -c -O2 -o SDL2_rotozoom.o ../SDL2_rotozoom.c
call emcc -s USE_SDL=2 -c -O2 -o SDL2_imageFilter.o ../SDL2_imageFilter.c

echo Linking to libSDL2_gfx.a...
echo %cd%
call emar rc libSDL2_gfx.a ^
	SDL2_framerate.o ^
	SDL2_gfxPrimitives.o ^
	SDL2_rotozoom.o ^
	SDL2_imageFilter.o

cd .. && mkdir bin & cd bin
mkdir include
mkdir include\SDL2
mkdir lib

copy /Y ..\SDL2_framerate.h include\SDL2\
copy /Y ..\SDL2_gfxPrimitives.h include\SDL2\
copy /Y ..\SDL2_gfxPrimitives_font.h include\SDL2\
copy /Y ..\SDL2_imageFilter.h include\SDL2\
copy /Y ..\SDL2_rotozoom.h include\SDL2\

copy /Y ..\build\libSDL2_gfx.a lib\

popd

echo -I%SDL2_gfx_src%\bin\include
echo -L%SDL2_gfx_src%\bin\lib -lSDL2_gfx

