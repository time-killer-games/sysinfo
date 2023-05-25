#!/bin/sh
cd "${0%/*}"
if [ $(uname) = "Darwin" ]; then
  clang++ main.cpp sysinfo.cpp libGLEW.a libglfw3.a -o sysinfo -std=c++17 -DGLEW_STATIC -I/usr/local/include `pkg-config --cflags glfw3` `pkg-config --cflags glew` -framework Cocoa -framework IOKit -framework OpenGL -lobjc -arch arm64 -arch x86_64; ./sysinfo;
elif [ $(uname) = "Linux" ]; then
  clang++ main.cpp sysinfo.cpp -o sysinfo -std=c++17 `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs glew` -lGL; ./sysinfo;
elif [ $(uname) = "FreeBSD" ]; then
  clang++ main.cpp sysinfo.cpp -o sysinfo -std=c++17 `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs glew` -lGL; ./sysinfo;
elif [ $(uname) = "DragonFly" ]; then
  clang++ main.cpp sysinfo.cpp -o sysinfo -std=c++17 `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs glew` -lGL; ./sysinfo;
elif [ $(uname) = "NetBSD" ]; then
  clang++ main.cpp sysinfo.cpp -o sysinfo -std=c++17 `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs glew` -lGL; ./sysinfo;
elif [ $(uname) = "OpenBSD" ]; then
  clang++ main.cpp sysinfo.cpp -o sysinfo -std=c++17 `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs glew` -lGL; ./sysinfo;
elif [ $(uname) = "SunOS" ]; then
  clang++ main.cpp sysinfo.cpp -o sysinfo -std=c++17 `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs glew` -lGL; ./sysinfo;
else
  g++ main.cpp sysinfo.cpp -o sysinfo.exe -std=c++17 -DGLEW_STATIC -static-libgcc -static-libstdc++ -static -lws2_32 -lglfw3 -lglew32 -lopengl32 -lgdi32 -ld3d11 -ldxgi; ./sysinfo.exe;
fi
