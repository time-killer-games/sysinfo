#!/bin/sh
pacman -Sy --noconfirm mingw-w64-i686-glew mingw-w64-x86_64-glew mingw-w64-i686-glfw mingw-w64-x86_64-glfw;
cd /c/projects/ween; g++ main.cpp sysinfo.cpp -o sysinfo.exe -std=c++17 -DGLEW_STATIC -static-libgcc -static-libstdc++ -static -lws2_32 -lglfw3 -lglew32 -lopengl32 -lgdi32 -ld3d11 -ldxgi; ./sysinfo.exe;
