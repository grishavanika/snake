#!/bin/sh

source ./0_append_distro_path.sh

untar_file SDL2-2.0.7.tar
untar_file SDL2_gfx-1.0.4.tar

patch -d /c/temp/gcc/SDL2-2.0.7 -p1 < sdl-clipcursor.patch

cd /c/temp/gcc

mv SDL2-2.0.7 src
mkdir build dest
cd build

../src/configure --build=x86_64-w64-mingw32 --host=x86_64-w64-mingw32 --target=x86_64-w64-mingw32 \
--prefix=/c/temp/gcc/dest --disable-shared

make $X_MAKE_JOBS all "CFLAGS=-s -O3"
make install
cd /c/temp/gcc
rm -rf build src

mv SDL2_gfx-1.0.4 src
mkdir build
cd build

#
 