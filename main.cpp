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
  "OS DEVICE NAME: " << ((!os_device_name().empty()) ? os_device_name() : "(null)") << "\n" <<
  "OS PRODUCT NAME: " << ((!os_product_name().empty()) ? os_product_name() : "(null)") << "\n" <<
  "OS KERNEL NAME: " << ((!os_kernel_name().empty()) ? os_kernel_name() : "(null)") << "\n" <<
  "OS KERNEL RELEASE: " << ((!os_kernel_release().empty()) ? os_kernel_release() : "(null)") << "\n" <<
  "OS KERNEL VERSION: " << ((!os_kernel_version().empty()) ? os_kernel_version() : "(null)") << "\n" <<
  "OS ARCHITECTURE: " << ((!os_architecture().empty()) ? os_architecture() : "(null)") << "\n" <<
  "CPU PROCESSOR: " << ((!cpu_processor().empty()) ? cpu_processor() : "(null)") << "\n" <<
  "CPU VENDOR: " << ((!cpu_vendor().empty()) ? cpu_vendor() : "(null)") << "\n" <<
  "CPU CORE COUNT: " << ((cpu_core_count() != -1) ? std::to_string(cpu_core_count()) : "(null)") << "\n" <<
  "CPU PROCESSOR COUNT: " << ((cpu_processor_count() != -1) ? std::to_string(cpu_processor_count()) : "(null)") << "\n" <<
  "RANDOM-ACCESS MEMORY TOTAL: " << ((!memory_totalram(true).empty()) ? memory_totalram(true) : "(null)") << "\n" <<
  "RANDOM-ACCESS MEMORY USED: " << ((!memory_usedram(true).empty()) ? memory_usedram(true) : "(null)") << "\n" <<
  "RANDOM-ACCESS MEMORY FREE: " << ((!memory_freeram(true).empty()) ? memory_freeram(true) : "(null)") << "\n" <<
  "SWAP MEMORY TOTAL: " << ((!memory_totalswap(true).empty()) ? memory_totalswap(true) : "(null)") << "\n" <<
  "SWAP MEMORY USED: " << ((!memory_usedswap(true).empty()) ? memory_usedswap(true) : "(null)") << "\n" <<
  "SWAP MEMORY FREE: " << ((!memory_freeswap(true).empty()) ? memory_freeswap(true) : "(null)") << "\n" <<
  "GPU MANUFACTURER: " << ((!gpu_manufacturer().empty()) ? gpu_manufacturer() : "(null)") << "\n" <<
  "GPU RENDERER: " << ((!gpu_renderer().empty()) ? gpu_renderer() : "(null)") << "\n" <<
  "GPU MEMORY: " << ((!memory_totalvram(true).empty()) ? memory_totalvram(true) : "(null)") << "\n";
  return 0;
}
