#!bin/sh
pacman -Sy --noconfirm mingw-w64-i686-glew mingw-w64-x86_64-glew;
cd /c/projects/ween; g++ libsysinfo.cpp -o sysinfo.exe -std=c++14 -DGLEW_STATIC -static-libgcc -static-libstdc++ -static -lws2_32 -lglew32 -lopengl32 -lgdi32; ./sysinfo.exe;
