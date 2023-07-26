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
  "OS DEVICE NAME: " << ((!utsname_nodename().empty()) ? utsname_nodename() : "(null)") << "\n" <<
  "OS PRODUCT NAME: " << ((!utsname_codename().empty()) ? utsname_codename() : "(null)") << "\n" <<
  "OS KERNEL NAME: " << ((!utsname_sysname().empty()) ? utsname_sysname() : "(null)") << "\n" <<
  "OS KERNEL RELEASE: " << ((!utsname_release().empty()) ? utsname_release() : "(null)") << "\n" <<
  "OS KERNEL VERSION: " << ((!utsname_version().empty()) ? utsname_version() : "(null)") << "\n" <<
  "OS ARCHITECTURE: " << ((!utsname_machine().empty()) ? utsname_machine() : "(null)") << "\n" <<
  "CPU PROCESSOR: " << ((!cpu_brand().empty()) ? cpu_brand() : "(null)") << "\n" <<
  "CPU VENDOR: " << ((!cpu_vendor().empty()) ? cpu_vendor() : "(null)") << "\n" <<
  "CPU CORE COUNT: " << ((cpu_numcores() != -1) ? std::to_string(cpu_numcores()) : "(null)") << "\n" <<
  "CPU PROCESSOR COUNT: " << ((cpu_numcpus() != -1) ? std::to_string(cpu_numcpus()) : "(null)") << "\n" <<
  "RANDOM-ACCESS MEMORY TOTAL: " << ((memory_totalram() != -1) ? human_readable(memory_totalram()) : "(null)") << "\n" <<
  "RANDOM-ACCESS MEMORY USED: " << ((memory_usedram() != -1) ? human_readable(memory_usedram()) : "(null)") << "\n" <<
  "RANDOM-ACCESS MEMORY FREE: " << ((memory_availram() != -1) ? human_readable(memory_availram()) : "(null)") << "\n" <<
  "SWAP MEMORY TOTAL: " << ((memory_totalvmem() != -1) ? human_readable(memory_totalvmem()) : "(null)") << "\n" <<
  "SWAP MEMORY USED: " << ((memory_usedvmem() != -1) ? human_readable(memory_usedvmem()) : "(null)") << "\n" <<
  "SWAP MEMORY FREE: " << ((memory_availvmem() != -1) ? human_readable(memory_availvmem()) : "(null)") << "\n" <<
  "GPU MANUFACTURER: " << ((!gpu_vendor().empty()) ? gpu_vendor() : "(null)") << "\n" <<
  "GPU RENDERER: " << ((!gpu_renderer().empty()) ? gpu_renderer() : "(null)") << "\n" <<
  "GPU MEMORY: " << ((gpu_videomemory() != -1) ? human_readable(gpu_videomemory()) : "(null)") << "\n";
  return 0;
}
