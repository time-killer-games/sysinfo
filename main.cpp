/*

 MIT License
 
 Copyright © 2023 Samuel Venable
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
*/

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#if (defined(__APPLE__) && defined(__MACH__))
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <iostream>

#include "sysinfo.hpp"

static GLFWwindow *window = nullptr;
static void create_opengl_context() {
  if (!window) {
    glewExperimental = true;
    if (!glfwInit()) return;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    window = glfwCreateWindow(1, 1, "", nullptr, nullptr);
    if (!window) return;
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) return;
  }
}

int main() {
  using namespace ngs::sys;
  create_opengl_context();
  std::cout <<
  "System Name: " << utsname_sysname() << "\n" <<
  "Host Name: " << utsname_nodename() << "\n" <<
  "OS Release: " << utsname_release() << "\n" <<
  "OS Version: " << utsname_version() << "\n" <<
  "OS Architecture: " << utsname_machine() << "\n" <<
  "Total RAM: " << human_readable(memory_totalram()) << "\n" <<
  "Free RAM: " << human_readable(memory_availram()) << "\n" <<
  "Used RAM: " << human_readable(memory_usedram()) << "\n" <<
  "Total Virtual Memory: " << human_readable(memory_totalvmem()) << "\n" <<
  "Free Virtual Memory: " << human_readable(memory_availvmem()) << "\n" <<
  "Used Virtual Memory: " << human_readable(memory_usedvmem()) << "\n" <<
  "GPU Vendor: " << gpu_vendor() << "\n" <<
  "GPU Renderer: " << gpu_renderer() << "\n" <<
  "Dedicated Video Memory: " << human_readable(gpu_videomemory()) << "\n" <<
  "CPU Vendor: " << cpu_vendor() << "\n" <<
  "CPU Brand String: " << cpu_brand() << "\n" <<
  "Number of CPU Cores: " << std::to_string(cpu_numcpus()) << "\n";
  return 0;
}

