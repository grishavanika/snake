@echo off
set vs_path="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"
set vcpkg_includes=C:\libs\vcpkg\installed\x64-windows\include
set vcpkg_libs=C:\libs\vcpkg\installed\x64-windows\lib

call %vs_path%\vcvars64.bat

cl.exe /permissive- /W4 /O2 /WX /MD /EHsc /nologo /std:c++17 snake.cpp ^
	/I%vcpkg_includes% %vcpkg_libs%\SDL2.lib %vcpkg_libs%\SDL2_gfx.lib ^
	/link /SUBSYSTEM:WINDOWS /MACHINE:X64 /OUT:snake_msvc.exe
