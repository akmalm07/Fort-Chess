@echo off

REM Create build folder if it doesn't exist

cd ..

if not exist build (
    mkdir build
)

REM Compile to build folder
em++ src/main.cpp src/game.cpp src/engine.cpp src/websockets.cpp ^
    -std=c++23 -o build/index.html ^
    -s ASYNCIFY -s USE_WEBGL2=1 -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH ^
    -I../vendors/raylib/include -I../global -I../include ^
    -L../vendors/raylib/lib -lraylib -lwebsocket.js -include headers.h

REM Run from build folder
cd build
emrun index.html
cd ..