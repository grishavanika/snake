@echo off

:: How to install emscripten
:: 
::cd /d C:\dev\_opensource
::git clone https://github.com/emscripten-core/emsdk.git
::cd emsdk
::emsdk install latest
::emsdk activate latest
::
:: Open NEW cmd and then:
::cd /d C:\dev\_opensource\emsdk
::emsdk_env.bat
:: 

:: Docs: https://emscripten.org/docs/api_reference/emscripten.h.html
:: SDL2 example: https://main.lv/writeup/web_assembly_sdl_example.md
:: 


call build_emscripten_SDL2_gfx.bat

set em_root=C:\dev\_opensource\emsdk
set SDL_gfx=C:\libs\SDL2_gfx-1.0.4

cd /d %em_root%
call emsdk_env.bat
cd /d %~dp0

echo Building ...

call emcc snake.cpp ^
	-O2 ^
	-s USE_SDL=2 ^
	-I%SDL_gfx%\bin\include ^
	-L%SDL_gfx%\bin\lib -lSDL2_gfx ^
	-std=c++14 ^
	-o snake_win.js

echo Running...
::emrun --no_browser --port 8080 .
::snake_win.html
emrun snake_win.html
