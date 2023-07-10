#!/bin/sh
cd "${0%/*}"
if [ $(uname) = "Darwin" ]; then
  clang++ /opt/local/lib/libSDL2.a main.cpp system.cpp -o sysinfo -Iinclude -L. -I/opt/local/include -I/opt/local/include/SDL2 -std=c++17 -DCREATE_CONTEXT -lobjc -liconv -lEGL -lGLESv2 -Wl,-framework,CoreAudio -Wl,-framework,AudioToolbox -Wl,-weak_framework,CoreHaptics -Wl,-weak_framework,GameController -Wl,-framework,ForceFeedback -lobjc -Wl,-framework,CoreVideo -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,IOKit -Wl,-weak_framework,QuartzCore -Wl,-weak_framework,Metal `pkg-config --cflags glfw3` `pkg-config --cflags glew` -fPIC -arch x86_64 -arch arm64; install_name_tool -change @rpath/libEGL.dylib @executable_path/libEGL.dylib ./sysinfo; install_name_tool -change @rpath/libGLESv2.dylib @executable_path/libGLESv2.dylib ./sysinfo; ./sysinfo;
elif [ $(uname) = "Linux" ]; then
  g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT -static-libgcc -static-libstdc++ `pkg-config --cflags --libs sdl2` -lGL; ./sysinfo;
elif [ $(uname) = "FreeBSD" ]; then
  clang++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT `pkg-config --cflags --libs sdl2` -lGL -lkvm; ./sysinfo;
elif [ $(uname) = "DragonFly" ]; then
  g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT -static-libgcc -static-libstdc++ `pkg-config --cflags --libs sdl2` -lGL -lkvm; ./sysinfo;
elif [ $(uname) = "NetBSD" ]; then
  g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT -static-libgcc -static-libstdc++ `pkg-config --cflags --libs sdl2` -lGL; ./sysinfo;
elif [ $(uname) = "OpenBSD" ]; then
  clang++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT `pkg-config --cflags --libs sdl2` -lGL; ./sysinfo;
elif [ $(uname) = "SunOS" ]; then
  export PKG_CONFIG_PATH=/usr/lib/64/pkgconfig && g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT -static-libgcc `pkg-config --cflags --libs sdl2` -lGL; ./sysinfo;
else
  g++ main.cpp system.cpp -o sysinfo.exe -std=c++17 -DCREATE_CONTEXT -Iinclude -static-libgcc -static-libstdc++ -static `pkg-config --cflags --libs --static sdl2` -lopengl32; ./sysinfo.exe;
fi
