#!/bin/sh
#!/bin/sh
cd "${0%/*}"
if [ $(uname) = "Darwin" ]; then
  clang++ main.cpp sysinfo.cpp libGLEW.a libglfw3.a -o sysinfo.app -std=c++17 -DGLEW_STATIC -I/usr/local/include `pkg-config --cflags glfw3` `pkg-config --cflags glew` -framework Cocoa -framework IOKit -framework OpenGL -lobjc -arch arm64 -arch x86_64; ./sysinfo.app;
else
  g++ main.cpp sysinfo.cpp -o sysinfo.exe -std=c++17 -DGLEW_STATIC -static-libgcc -static-libstdc++ -static -lws2_32 -lglfw3 -lglew32 -lopengl32 -lgdi32 -ld3d11 -ldxgi; ./sysinfo.exe;
fi
