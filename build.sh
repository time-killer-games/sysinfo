#!/bin/sh
cd "${0%/*}";
xxd -i pci.ids > pci.ids.hpp;
if [ $(uname) = "Darwin" ]; then
  clang++ main.cpp system.cpp -o sysinfo -std=c++17 -arch x86_64 -arch arm64; ./sysinfo;
elif [ $(uname) = "Linux" ]; then
  g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT -static-libgcc -static-libstdc++ `pkg-config --cflags --libs sdl2` `pkg-config --cflags --libs x11` -lGL; ./sysinfo;
elif [ $(uname) = "FreeBSD" ]; then
  clang++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT `pkg-config --cflags --libs sdl2` `pkg-config --cflags --libs x11` -lGL -lkvm; ./sysinfo;
elif [ $(uname) = "DragonFly" ]; then
  g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT -static-libgcc -static-libstdc++ `pkg-config --cflags --libs sdl2` `pkg-config --cflags --libs x11` -lGL -lkvm -lpthread; ./sysinfo;
elif [ $(uname) = "NetBSD" ]; then
  g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT -static-libgcc -static-libstdc++ `pkg-config --cflags --libs sdl2` `pkg-config --cflags --libs x11` -I/usr/X11R7/include -Wl,-rpath,/usr/X11R7/lib -L/usr/X11R7/lib -lGL; ./sysinfo;
elif [ $(uname) = "OpenBSD" ]; then
  clang++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT `pkg-config --cflags --libs sdl2` `pkg-config --cflags --libs x11` -lGL; ./sysinfo;
elif [ $(uname) = "SunOS" ]; then
  export PKG_CONFIG_PATH=/usr/lib/64/pkgconfig && g++ main.cpp system.cpp -o sysinfo -std=c++17 -DCREATE_CONTEXT -static-libgcc `pkg-config --cflags --libs sdl2` `pkg-config --cflags --libs x11` -lGL; ./sysinfo;
else
  g++ main.cpp system.cpp -o sysinfo.exe -std=c++17 -static-libgcc -static-libstdc++ -lws2_32 -ldxgi -lpsapi -static; ./sysinfo.exe;
fi
