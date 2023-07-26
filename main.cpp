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

#if defined(_WIN32)
#undef main
#endif

using namespace ngs::sys;

int main() {
  std::cout <<
  "OS DEVICE NAME: " << utsname_nodename() << "\n" <<
  "OS PRODUCT NAME: " << utsname_codename() << "\n" <<
  "OS ARCHITECTURE: " << utsname_machine() << "\n" <<
  "KERNEL NAME: " << utsname_sysname() << "\n" <<
  "KERNEL RELEASE: " << utsname_release() << "\n" <<
  "CPU PROCESSOR: " << cpu_brand() << "\n" <<
  "CPU VENDOR: " << cpu_vendor() << "\n" <<
  "CPU CORE COUNT: " << std::to_string(cpu_numcores()) << "\n" <<
  "CPU PROCESSOR COUNT: " << std::to_string(cpu_numcpus()) << "\n" <<
  "RANDOM-ACCESS MEMORY TOTAL: " << human_readable(memory_totalram()) << "\n" <<
  "RANDOM-ACCESS MEMORY USED: " << human_readable(memory_usedram()) << "\n" <<
  "RANDOM-ACCESS MEMORY FREE: " << human_readable(memory_availram()) << "\n" <<
  "SWAP MEMORY TOTAL: " << human_readable(memory_totalvmem()) << "\n" <<
  "SWAP MEMORY USED: " << human_readable(memory_usedvmem()) << "\n" <<
  "SWAP MEMORY FREE: " << human_readable(memory_availvmem()) << "\n" <<
  "GPU MANUFACTURER: " << ((!gpu_vendor().empty()) ? gpu_vendor() : "(null)") << "\n" <<
  "GPU RENDERER: " << ((!gpu_renderer().empty()) ? gpu_renderer() : "(null)") << "\n" <<
  "GPU MEMORY: " << ((gpu_videomemory() > 0) ? human_readable(gpu_videomemory()) : "(null)") << "\n";
  return 0;
}
