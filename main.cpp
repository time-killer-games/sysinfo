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

#include <iostream>

#include "system.hpp"

using namespace ngs::sys;

int main() {
  std::cout <<
  "Product Name: " << utsname_codename() << "\n" <<
  "Host Name: " << utsname_nodename() << "\n" <<
  "OS Kernel: " << utsname_sysname() << "\n" <<
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
  "Number of CPU Cores: " << std::to_string(cpu_numcores()) << "\n" <<
  "Number of CPU Processors: " << std::to_string(cpu_numcpus()) << "\n";
  return 0;
}

