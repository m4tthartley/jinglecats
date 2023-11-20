#!/usr/bin/bash
clear

mkdir -p build
pushd build

echo "Compiling JingleCats..."

opts="-g -Wno-incompatible-pointer-types"
# code = $cd

files=../main.c
gcc $opts $files -o jinglecats.exe -luser32 -lgdi32 -lopengl32

files=../jinglecats.c
gcc $opts $files -o _game.dll -shared -luser32 -lgdi32 -lopengl32 -lmingw32
# /DLL  /EXPORT:gamestart /EXPORT:gameloop 

popd