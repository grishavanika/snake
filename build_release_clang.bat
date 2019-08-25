@echo off
set vcpkg_includes=C:\libs\vcpkg\installed\x64-windows\include
set vcpkg_libs=C:\libs\vcpkg\installed\x64-windows\lib
set clang_cl=C:\Programs\LLVM\bin\clang-cl.exe

%clang_cl% -Wno-pragma-pack ^
	/W4 /O2 /WX /MD /EHsc /nologo /std:c++17 snake.cpp ^
	/I%vcpkg_includes% %vcpkg_libs%\SDL2.lib %vcpkg_libs%\SDL2_gfx.lib ^
	/link /SUBSYSTEM:WINDOWS /MACHINE:X64 /OUT:snake_clang.exe
