@echo off

sh ./build.sh
exit 0

set opts=-FC -nologo -Zi -W2
set code=%cd%
pushd build

set files=%code%\main.c
cl %opts% %files% -Fejinglecats.exe user32.lib gdi32.lib opengl32.lib

set /A PDB_NUMBER=PDB_NUMBER+1

set files=%code%\jinglecats.c
cl %opts% %files% -Fe_game.dll user32.lib gdi32.lib opengl32.lib /LD /link /PDB:game%random%.pdb /DLL /EXPORT:gamestart /EXPORT:gameloop 

popd
