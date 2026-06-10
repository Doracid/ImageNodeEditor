@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set "Qt5_DIR=C:\Users\sunli\anaconda3\Library\lib\cmake\Qt5"
cmake -S "%~dp0" -B "%~dp0build" -G "Visual Studio 17 2022" -A x64
cmake --build "%~dp0build" --config Release
echo Done.
