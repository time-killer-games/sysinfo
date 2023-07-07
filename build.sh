#!/bin/sh
cd "${0%/*}"
if [ $(uname) = "Darwin" ]; then
  clang++ main.cpp system.cpp libGLEW.a libglfw3.a -o sysinfo -std=c++17 -DCREATE_OPENGL_CONTEXT -DGLEW_STATIC -Iinclude `pkg-config --cflags glfw3` `pkg-config --cflags glew` -framework Cocoa -framework IOKit -framework OpenGL -lobjc -arch arm64 -arch x86_64; ./sysinfo;
elif [ $(uname) = "Linux" ]; then
  g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_OPENGL_CONTEXT -static-libgcc -static-libstdc++ `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs glew` -lGL; ./sysinfo;
elif [ $(uname) = "FreeBSD" ]; then
  clang++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_OPENGL_CONTEXT `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs glew` -lGL -lkvm; ./sysinfo;
elif [ $(uname) = "DragonFly" ]; then
  g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_OPENGL_CONTEXT -static-libgcc -static-libstdc++ `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs glew` -lGL -lkvm; ./sysinfo;
elif [ $(uname) = "NetBSD" ]; then
  g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_OPENGL_CONTEXT -static-libgcc -static-libstdc++ `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs glew` -lGL; ./sysinfo;
elif [ $(uname) = "OpenBSD" ]; then
  clang++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_OPENGL_CONTEXT `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs glew` -lGL; ./sysinfo;
elif [ $(uname) = "SunOS" ]; then
  export PKG_CONFIG_PATH=/usr/lib/64/pkgconfig && g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_OPENGL_CONTEXT -static-libgcc `pkg-config --cflags --libs sdl2` `pkg-config --cflags --libs glew` -lGL; ./sysinfo;
else
  g++ main.cpp system.cpp -o sysinfo.exe -std=c++17 -DCREATE_OPENGL_CONTEXT -DGLEW_STATIC -static-libgcc -static-libstdc++ -static -lws2_32 -ldxgi; ./sysinfo.exe;
fi
