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

#if (defined(_WIN32) && defined(_MSC_VER))
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <thread>
#include <regex>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <psapi.h>
#include <dxgi.h>
#else
#if (defined(__APPLE__) && defined(__MACH__))
#include <mach/host_info.h>
#include <mach/mach_host.h>
#include <mach/mach_time.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <libproc.h>
#include <unistd.h>
#elif defined(__linux__)
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
#if defined(CREATE_CONTEXT)
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#endif
#endif
#if (defined(_WIN32) && defined(_MSC_VER))
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "dxgi.lib")
#endif
#include "pci.ids.hpp"
#if defined(_WIN32)
#define EXPORTED_FUNCTION extern "C" __declspec(dllexport)
#else
#define EXPORTED_FUNCTION extern "C" __attribute__((visibility("default")))
#endif

namespace {

#if (!defined(_WIN32) && (!defined(__APPLE__) && !defined(__MACH__)))
#if defined(CREATE_CONTEXT)
SDL_Window *window = nullptr;
#endif
#endif
#if defined(_WIN32)
std::string windows_product_name;
std::string windows_version_number;
#endif
std::string devicename;
bool devicenameerror = false;
std::string productname;
bool productnameerror = false;
std::string kernelname;
bool kernelnameerror = false;
std::string kernelrelease;
bool kernelreleaseerror = false;
std::string kernelversion;
bool kernelversionerror = false;
std::string architecture;
bool architectureerror = false;
unsigned device_id = 0;
std::unordered_map<unsigned, std::string> vendor_name_by_id;
unsigned vendor_id = 0;
std::unordered_map<unsigned, std::string> device_name_by_id;
std::string gpuvendor;
bool gpuvendorerror = false;
std::string gpurenderer;
bool gpurenderererror = false;
long long videomemory = -1;
bool videomemoryerror = false;
std::string cpuvendor;
bool cpuvendorerror = false;
std::string cpubrand;
bool cpubranderror = false;
int numcores = -1;
bool numcoreserror = false;
int numcpus = -1;
bool numcpuserror = false;
long long totalram = -1;
bool totalramerror = false;
#if defined(_WIN32)
std::string wine_version;
#endif

struct HumanReadable {
  long double size{};
  private: friend
  std::ostream& operator<<(std::ostream& os, HumanReadable hr) {
    int i{};
    long double mantissa = hr.size;
    for (; mantissa >= 1024; mantissa /= 1024, i++) { }
    mantissa = std::ceil(mantissa * 100) / 100;
    os << mantissa << " " << "BKMGTPE"[i];
    return i == 0 ? os : os << "B";
  }
};

struct HumanReadableRoundUp {
  long double size{};
  private: friend
  std::ostream& operator<<(std::ostream& os, HumanReadableRoundUp hr) {
    int i{};
    long double mantissa = hr.size;
    for (; mantissa >= 1024; mantissa /= 1024, i++) { }
    mantissa = std::ceil(mantissa * 1000000000000000) / 1000000000000000;
    os << mantissa << " " << "BKMGTPE"[i];
    return i == 0 ? os : os << "B";
  }
};

std::string human_readable(long double nbytes, bool roundup) {
  if (nbytes == -1) return "";
  std::stringstream ss;
  if (roundup) {
    ss << HumanReadableRoundUp{nbytes};
    std::size_t pos1 = ss.str().find(".");
    std::size_t pos2 = ss.str().find(" ");
    if (pos1 != std::string::npos && pos2 != std::string::npos) {
      std::string sfx = ss.str().substr(pos2, pos2 + 1);
      long double numb = strtod(ss.str().c_str(), nullptr);
      return std::to_string((long long)numb + 1) + sfx;
    }
  } else {
    ss << HumanReadable{nbytes};
  }
  return ss.str();
}

/* Define CREATE_CONTEXT in your build scripts or Makefiles if
the calling process hasn't already done this on its own ... */
#if (!defined(_WIN32) && (!defined(__APPLE__) && !defined(__MACH__)))
#if defined(CREATE_CONTEXT)
bool create_context() {
  if (!window) {
    SDL_setenv("SDL_VIDEODRIVER", "x11", 1);
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    if (SDL_Init(SDL_INIT_VIDEO))
      return false;
    window = SDL_CreateWindow("", 0, 0, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (!window) 
      return false;
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) 
      return false;
    int err = SDL_GL_MakeCurrent(window, context);
    if (err) 
      return false;
  }
  return true;
}
#endif
#endif

#if defined(_WIN32)
void message_pump() {
  MSG msg;
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

std::string wine_get_version() {
  if (!wine_version.empty())
    return wine_version;
  static const char *(CDECL *pwine_get_version)(void);
  HMODULE hntdll = GetModuleHandle("ntdll.dll");
  if (!hntdll)
    return "";
  pwine_get_version = (const char *(*)())GetProcAddress(hntdll, "wine_get_version");
  if (!pwine_get_version)
    return "";
  return pwine_get_version();
}
#endif

std::string read_output(std::string cmd) {
  std::string result;
  #if defined(_WIN32)
  bool proceed = true;
  HANDLE stdin_read = nullptr;
  HANDLE stdin_write = nullptr;
  HANDLE stdout_read = nullptr; 
  HANDLE stdout_write = nullptr;
  SECURITY_ATTRIBUTES sa = { 
    sizeof(SECURITY_ATTRIBUTES), 
    nullptr, 
    true 
  };
  proceed = CreatePipe(&stdin_read, &stdin_write, &sa, 0);
  if (!proceed)
    return "";
  SetHandleInformation(stdin_write, HANDLE_FLAG_INHERIT, 0);
  proceed = CreatePipe(&stdout_read, &stdout_write, &sa, 0);
  if (!proceed)
    return "";
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(STARTUPINFO);
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdError = stdout_write;
  si.hStdOutput = stdout_write;
  si.hStdInput = stdin_read;
  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));
  std::vector<char> ccmd;
  ccmd.resize(cmd.length() + 1, '\0');
  strncpy_s(&ccmd[0], cmd.length() + 1, cmd.c_str(), cmd.length() + 1);
  BOOL success = CreateProcessA(nullptr, &ccmd[0], nullptr, nullptr, true, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
  if (success) {
    DWORD nRead = 0;
    char buffer[1024];
    CloseHandle(stdout_write);
    CloseHandle(stdin_read);
    HANDLE wait_handles[] = { 
      pi.hProcess,
      stdout_read
    };
    while (MsgWaitForMultipleObjects(2, wait_handles, false, 5, QS_ALLEVENTS) != WAIT_OBJECT_0) {
      message_pump();
      while (ReadFile(stdout_read, buffer, 1024, &nRead, nullptr) && nRead) {
        message_pump();
        buffer[nRead] = '\0';
        result.append(buffer, nRead);
      }
      // remove trailing whitespace and newlines we do not need in return strings
      while (!result.empty() && (result.back() == ' ' || result.back() == '\t' ||
        result.back() == '\r' || result.back() == '\n')) {
        message_pump();
        result.pop_back();
      }
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(stdout_read);
    CloseHandle(stdin_write);
  }
  #else
  char buf[1024];
  ssize_t nRead = 0;
  FILE *fp = popen(cmd.c_str(), "r");
  if (fp) {
    while ((nRead = read(fileno(fp), buf, 1024)) > 0) {
      buf[nRead] = '\0';
      result.append(buf, nRead);
    }
    // remove trailing whitespace and newlines we do not need in return strings
    while (!result.empty() && (result.back() == ' ' || result.back() == '\t' ||
      result.back() == '\r' || result.back() == '\n'))
      result.pop_back();
    pclose(fp);
  }
  #endif
  static std::string str;
  str = result;
  return str;
}

#if defined(_WIN32)
void allocate_windows_version_number_and_product_name() {
  auto GetOSMajorVersionNumber = []() {
    int val = 0;
    char buf[1024];
    static std::string str;
    DWORD sz = sizeof(val);
    if (RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "CurrentMajorVersionNumber", RRF_RT_REG_DWORD, nullptr, &val, &sz) == ERROR_SUCCESS) {
      if (sprintf(buf, "%d", val) != -1) {
        str = buf;
      }
    }
    return str;
  };
  auto GetOSMinorVersionNumber = []() {
    int val = 0;
    char buf[1024];
    static std::string str;
    DWORD sz = sizeof(val);
    if (RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "CurrentMinorVersionNumber", RRF_RT_REG_DWORD, nullptr, &val, &sz) == ERROR_SUCCESS) {
      if (sprintf(buf, "%d", val) != -1) {
        str = buf;
      }
    }
    return str;
  };
  auto GetOSBuildNumber = []() {
    char buf[1024];
    static std::string str;
    DWORD sz = sizeof(buf);
    if (RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "CurrentBuildNumber", RRF_RT_REG_SZ, nullptr, &buf, &sz) == ERROR_SUCCESS) {
      str = buf;
    }
    return str;
  };
  auto GetOSRevisionNumber = []() {
    int val = 0;
    char buf[1024];
    static std::string str;
    DWORD sz = sizeof(val);
    if (RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "UBR", RRF_RT_REG_DWORD, nullptr, &val, &sz) == ERROR_SUCCESS) {
      if (sprintf(buf, "%d", val) != -1) {
        str = buf;
      }
    }
    return str;
  };
  char buf[1024];
  if (!GetOSMajorVersionNumber().empty() && !GetOSMinorVersionNumber().empty() && !GetOSBuildNumber().empty() && !GetOSRevisionNumber().empty()) {
    if (sprintf(buf, "%s.%s.%s.%s", GetOSMajorVersionNumber().c_str(), GetOSMinorVersionNumber().c_str(), GetOSBuildNumber().c_str(), GetOSRevisionNumber().c_str()) != -1) {
      windows_version_number = buf;
      char buf[1024];
      DWORD sz = sizeof(buf);
      if (RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "ProductName", RRF_RT_REG_SZ, nullptr, &buf, &sz) == ERROR_SUCCESS) {
        if (strtoull(GetOSMajorVersionNumber().c_str(), nullptr, 10) == 10 && strtoull(GetOSBuildNumber().c_str(), nullptr, 10) >= 22000) {
          std::string tmp = buf;
          if (!tmp.empty()) {
            tmp = std::regex_replace(tmp, std::regex("10"), "11");
            windows_product_name = tmp;
          }
        } else {
          windows_product_name = buf;
        }
      }
    }
  }
}
#endif

std::vector<std::string> string_split(std::string str, char delimiter) {
  std::vector<std::string> vec;
  std::stringstream sstr(str);
  std::string tmp;
  while (std::getline(sstr, tmp, delimiter)) {
    #if defined(_WIN32)
    message_pump();
    #endif
    vec.push_back(tmp);
  }
  return vec;
}

std::string get_vendor_or_device_name_by_id(unsigned identifier, bool vendor_or_device) {
  if (vendor_name_by_id.find(identifier) != vendor_name_by_id.end() && !vendor_or_device)
    return vendor_name_by_id[identifier];
  if (device_name_by_id.find(identifier) != device_name_by_id.end() && vendor_or_device)
    return device_name_by_id[identifier];
  std::string str(pci_ids, pci_ids + sizeof(pci_ids) / sizeof(pci_ids[0]));
  str = std::regex_replace(str, std::regex("\r"), "");
  std::vector<std::string> vec = string_split(str, '\n');
  for (std::size_t i = 0; i < vec.size(); i++) {
    #if defined(_WIN32)
    message_pump();
    #endif
    if (vec[i].empty() || (!vec[i].empty() && vec[i][0] == '#'))
      continue;
    std::size_t pos1 = vec[i].find(" ");
    if (pos1 != std::string::npos && vec[i][0] != '\t') {
      std::istringstream converter1(vec[i].substr(0, pos1));
      converter1 >> std::hex >> vendor_id;
      vendor_name_by_id[vendor_id] = vec[i].substr(pos1 + 2);
    }
    if (vendor_name_by_id[vendor_id] != vendor_name_by_id[identifier])
      continue;
    std::size_t pos2 = vec[i].find("\t\t");
    if (pos2 == std::string::npos && vec[i][0] == '\t') {
      std::size_t pos3 = vec[i].find(" ");
      if (pos3 != std::string::npos) {
        std::istringstream converter2(vec[i].substr(1, 4));
        converter2 >> std::hex >> device_id;
        device_name_by_id[device_id] = &vec[i].substr(pos3)[2];
      }
    }
  }
  return vendor_or_device ? device_name_by_id[identifier] : vendor_name_by_id[identifier];
}

std::string os_kernel_name() {
  if (!kernelname.empty())
    return kernelname;
  if (kernelnameerror)
    return "";
  #if !defined(_WIN32)
  struct utsname name;
  if (!uname(&name))
    kernelname = name.sysname;
  #else
  char buf[1024];
  DWORD sz = sizeof(buf);
  if (RegGetValueA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\\", "OS", RRF_RT_REG_SZ, nullptr, &buf, &sz) == ERROR_SUCCESS)
    kernelname = buf;
  #endif
  if (!kernelname.empty())
    return kernelname;
  kernelnameerror = true;
  return "";
}

std::string os_device_name() {
  if (!devicename.empty())
    return devicename;
  if (devicenameerror)
    return "";
  #if !defined(_WIN32)
  struct utsname name;
  if (!uname(&name))
    devicename = name.nodename;
  #else
  char buf[1024];
  WSADATA data;
  WORD wVersionRequested;
  wVersionRequested = MAKEWORD(2, 2);
  if (!WSAStartup(wVersionRequested, &data)) {
    if (!gethostname(buf, sizeof(buf))) {
      devicename = buf;
    }
    WSACleanup();
  }
  #endif
  if (!devicename.empty())
    return devicename;
  devicenameerror = true;
  return "";
}

std::string os_kernel_version() {
  if (!kernelversion.empty())
    return kernelversion;
  if (kernelversionerror)
    return "";
  #if !defined(_WIN32)
  struct utsname name;
  if (!uname(&name))
    kernelversion = name.version;
  #else
  std::string tmp = read_output("cmd /c ver");
  if (!tmp.empty()) {
    tmp = std::regex_replace(tmp, std::regex("\t"), "");
    tmp = std::regex_replace(tmp, std::regex("\r"), "");
    tmp = std::regex_replace(tmp, std::regex("\n"), "");
    kernelversion = tmp;
  }
  #endif
  if (!kernelversion.empty())
    return kernelversion;
  kernelversionerror = true;
  return "";
}

std::string os_kernel_release() {
  if (!kernelrelease.empty())
    return kernelrelease;
  if (kernelreleaseerror)
    return "";
  #if !defined(_WIN32)
  struct utsname name;
  if (!uname(&name))
    kernelrelease = name.release;
  #else
  if (!windows_version_number.empty()) {
    kernelrelease = windows_version_number;
    return kernelrelease;
  }
  allocate_windows_version_number_and_product_name();
  kernelrelease = windows_version_number;
  wine_version = wine_get_version();
  if (!wine_version.empty()) {
    std::string tmp = os_kernel_version();
    if (!tmp.empty()) {
      tmp = std::regex_replace(tmp, std::regex("Microsoft Windows "), "");
      tmp = std::regex_replace(tmp, std::regex("v"), "");
      tmp = std::regex_replace(tmp, std::regex("V"), "");
      tmp = std::regex_replace(tmp, std::regex("\\["), "");
      tmp = std::regex_replace(tmp, std::regex("\\]"), "");
      kernelrelease = tmp;
    }
  }
  #endif
  if (!kernelrelease.empty())
    return kernelrelease;
  kernelreleaseerror = true;
  return "";
}

std::string os_product_name() {
  if (!productname.empty())
    return productname;
  if (productnameerror)
    return "";
  #if defined(_WIN32)
  if (!windows_product_name.empty()) {
    productname = windows_product_name;
    return productname;
  }
  allocate_windows_version_number_and_product_name();
  productname = windows_product_name;
  wine_version = wine_get_version();
  if (!wine_version.empty()) {
    productname = "wine-" + wine_version;
  }
  #elif (defined(__APPLE__) && defined(__MACH__))
  std::string tmp1 = read_output("echo $(sw_vers | grep 'ProductName:' | uniq | awk 'NR==1{$1=$1;print}' && sw_vers | grep 'ProductVersion:' | uniq | awk 'NR==1{$1=$1;print}')");
  if (!tmp1.empty()) {
    tmp1 = std::regex_replace(tmp1, std::regex("ProductName: "), "");
    tmp1 = std::regex_replace(tmp1, std::regex("ProductVersion: "), "");
    std::fstream doc;
    doc.open("/System/Library/CoreServices/Setup Assistant.app/Contents/Resources/en.lproj/OSXSoftwareLicense.rtf", std::ios::in);
    if (doc.is_open()) {
      std::string tmp2;
      while (std::getline(doc, tmp2)) {
        std::string tmp3 = tmp2;
        std::transform(tmp2.begin(), tmp2.end(), tmp2.begin(), ::toupper);
        std::size_t pos1 = tmp2.find("SOFTWARE LICENSE AGREEMENT FOR MAC OS X");
        std::size_t pos2 = tmp2.find("SOFTWARE LICENSE AGREEMENT FOR MACOS");
        std::size_t len1 = strlen("SOFTWARE LICENSE AGREEMENT FOR MAC OS X");
        std::size_t len2 = strlen("SOFTWARE LICENSE AGREEMENT FOR MACOS");
        if (pos1 != std::string::npos) {
          tmp1 += tmp3.substr(pos1 + len1);
          tmp1 = tmp1.substr(0, tmp1.length() - 1);
          break;
        } else if (pos2 != std::string::npos) {
          tmp1 += tmp3.substr(pos2 + len2);
          tmp1 = tmp1.substr(0, tmp1.length() - 1);
          break;
        }
      }
      doc.close();
    }
    productname = tmp1;
  }
  #elif defined(__linux__)
  std::string tmp = read_output("echo $(lsb_release --id 2> /dev/null && lsb_release --release 2> /dev/null && lsb_release --codename 2> /dev/null)");
  if (!tmp.empty()) {
    tmp = std::regex_replace(tmp, std::regex("\r"), "");
    tmp = std::regex_replace(tmp, std::regex("\n"), "");
    tmp = std::regex_replace(tmp, std::regex("Distributor ID: "), "");
    tmp = std::regex_replace(tmp, std::regex("Release: "), "");
    tmp = std::regex_replace(tmp, std::regex("Codename: "), "");
    productname = tmp;
  } else {
    /* if lsb_release is not installed use kernel + release: */
    productname = os_kernel_name() + " " + os_kernel_release();
  }
  #endif
  if (!productname.empty())
    return productname;
  productnameerror = true;
  return "";
}

std::string os_architecture() {
  if (!architecture.empty())
    return architecture;
  if (architectureerror)
    return "";
  #if !defined(_WIN32)
  /* utsname.machine equals the achitecture of the 
  current executable - not the current platform */
  architecture = read_output("uname -m");
  #else
  char buf[1024];
  DWORD sz = sizeof(buf);
  if (RegGetValueA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\\", "PROCESSOR_ARCHITECTURE", RRF_RT_REG_SZ, nullptr, &buf, &sz) == ERROR_SUCCESS)
    architecture = buf;
  #endif
  if (!architecture.empty())
    return architecture;
  architectureerror = true;
  return "";
}

long long memory_totalram() {
  if (totalram != -1)
    return totalram;
  if (totalramerror)
    return -1;
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex))
    totalram = (long long)statex.ullTotalPhys;
  #elif (defined(__APPLE__) && defined(__MACH__))
  int mib[2];
  long long buf = -1;
  mib[0] = CTL_HW;
  mib[1] = HW_MEMSIZE;
  std::size_t sz = sizeof(long long);
  if (!sysctl(mib, 2, &buf, &sz, nullptr, 0))
    totalram = buf;
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info))
    totalram = (info.totalram * info.mem_unit);
  #endif
  if (!totalram)
    totalram = -1;
  if (totalram != -1)
    return totalram;
  totalramerror = true;
  return -1;
}

long long memory_freeram() {
  long long freeram = -1;
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex))
    freeram = (long long)statex.ullAvailPhys;
  #elif (defined(__APPLE__) && defined(__MACH__))
  long page_s = sysconf(_SC_PAGESIZE);
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
  vm_statistics64_data_t vmstat;
  if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vmstat, &count) == KERN_SUCCESS) { 
    if (vmstat.free_count * (long long)page_s != 0)
      freeram = (long long)(vmstat.free_count * (long long)page_s);
  }
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info))
    freeram = (info.freeram * info.mem_unit);
  #endif
  if (freeram != -1)
    return freeram;
  return -1;
}

long long memory_usedram() {
  long long usedram = -1;
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex))
    usedram = (long long)(statex.ullTotalPhys - statex.ullAvailPhys);
  #elif (defined(__APPLE__) && defined(__MACH__))
  if (memory_totalram() != -1 && memory_freeram() != -1)
    usedram = memory_totalram() - memory_freeram();
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info))
    usedram = ((info.totalram - info.freeram) * info.mem_unit);
  #endif
  if (usedram != -1)
    return usedram;
  return -1;
}

std::string gpu_manufacturer() {
  if (!gpuvendor.empty())
    return gpuvendor;
  if (gpuvendorerror)
    return "";
  #if defined(_WIN32)
  IDXGIFactory *pFactory = nullptr;
  if (CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory) == S_OK) {
    IDXGIAdapter *pAdapter = nullptr;
    if (pFactory->EnumAdapters(0, &pAdapter) == S_OK) {
      DXGI_ADAPTER_DESC adapterDesc;
      if (pAdapter->GetDesc(&adapterDesc) == S_OK)
        gpuvendor = get_vendor_or_device_name_by_id(adapterDesc.VendorId, false);
      pAdapter->Release();
    }
    pFactory->Release();
  }
  if (!gpuvendor.empty())
    return gpuvendor;
  #elif (defined(__APPLE__) && defined(__MACH__))
  unsigned identifier = 0;
  std::istringstream converter(read_output("ioreg -bls | grep -n2 '    | |   | |   \"model\" = <\"' | awk -F',\"pci' 'NR==5{print $2}' | rev | cut -c 2- | rev | awk -F',' '{print $1}'"));
  converter >> std::hex >> identifier;
  gpuvendor = get_vendor_or_device_name_by_id(identifier, false);
  if (!gpuvendor.empty()) 
    return gpuvendor;
  gpuvendor = read_output("system_profiler SPDisplaysDataType | grep -i 'Vendor: ' | uniq | awk -F 'Vendor: ' 'NR==1{$1=$1;print}' | awk 'NR==1{$1=$1;print}'");
  std::string tmp = gpuvendor;
  std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
  if (!gpuvendor.empty() && tmp.find("APPLE") != std::string::npos)
    gpuvendor = "Apple";
  if (!gpuvendor.empty()) 
    return gpuvendor;
  #elif defined(__linux__)
  #if defined(CREATE_CONTEXT)
  if (!create_context()) {
    gpuvendorerror = true;
    return "";
  }
  #endif
  unsigned v = 0;
  PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC queryInteger;
  queryInteger = (PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC)glXGetProcAddressARB((const GLubyte *)"glXQueryCurrentRendererIntegerMESA");
  queryInteger(GLX_RENDERER_VENDOR_ID_MESA, &v);
  gpuvendor = v ? get_vendor_or_device_name_by_id(v, false) : "";
  if (!gpuvendor.empty())
    return gpuvendor;
  PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC queryString;
  queryString = (PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC)glXGetProcAddressARB((const GLubyte *)"glXQueryCurrentRendererStringMESA");
  const GLubyte *vendor = (GLubyte *)queryString(GLX_RENDERER_VENDOR_ID_MESA);
  gpuvendor = vendor ? (char *)vendor : "";
  if (!gpuvendor.empty())
    return gpuvendor;
  #endif
  gpuvendorerror = true;
  return "";
}

std::string gpu_renderer() {
  if (!gpurenderer.empty())
    return gpurenderer;
  if (gpurenderererror)
    return "";
  std::string result;
  #if defined(_WIN32)
  auto narrow = [](std::wstring wstr) {
    if (wstr.empty()) return std::string("");
    int nbytes = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), nullptr, 0, nullptr, nullptr);
    std::vector<char> buf(nbytes);
    return std::string { buf.data(), (std::size_t)WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), buf.data(), nbytes, nullptr, nullptr) };
  };
  IDXGIFactory *pFactory = nullptr;
  if (CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory) == S_OK) {
    IDXGIAdapter *pAdapter = nullptr;
    if (pFactory->EnumAdapters(0, &pAdapter) == S_OK) {
      DXGI_ADAPTER_DESC adapterDesc;
      if (pAdapter->GetDesc(&adapterDesc) == S_OK) {
        gpurenderer = get_vendor_or_device_name_by_id(adapterDesc.DeviceId, true);
        if (!gpurenderer.empty()) 
          return gpurenderer;
        gpurenderer = narrow(adapterDesc.Description);
        if (!gpurenderer.empty()) 
          return gpurenderer;
      }
      pAdapter->Release();
    }
    pFactory->Release();
  }
  #elif (defined(__APPLE__) && defined(__MACH__))
  unsigned identifier = 0;
  std::istringstream converter(read_output("ioreg -bls | grep -n2 '    | |   | |   \"model\" = <\"' | awk -F',\"pci' 'NR==5{print $2}' | rev | cut -c 2- | rev | awk -F',' '{print $2}'"));
  converter >> std::hex >> identifier;
  gpurenderer = get_vendor_or_device_name_by_id(identifier, true);
  if (!gpurenderer.empty()) 
    return gpurenderer;
  gpurenderer = read_output("system_profiler SPDisplaysDataType | grep -i 'Chipset Model: ' | uniq | awk -F 'Chipset Model: ' 'NR==1{$1=$1;print}' | awk 'NR==1{$1=$1;print}'");
  if (!gpurenderer.empty()) 
    return gpurenderer;
  #elif defined(__linux__)
  #if defined(CREATE_CONTEXT)
  if (!create_context()) {
    gpurenderererror = true;
    return "";
  }
  #endif
  unsigned v = 0;
  PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC queryInteger;
  queryInteger = (PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC)glXGetProcAddressARB((const GLubyte *)"glXQueryCurrentRendererIntegerMESA");
  queryInteger(GLX_RENDERER_DEVICE_ID_MESA, &v);
  gpurenderer = v ? get_vendor_or_device_name_by_id(v, true) : "";
  if (!gpurenderer.empty())
    return gpurenderer;
  PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC queryString;
  queryString = (PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC)glXGetProcAddressARB((const GLubyte *)"glXQueryCurrentRendererStringMESA");
  const GLubyte *renderer = (GLubyte *)queryString(GLX_RENDERER_DEVICE_ID_MESA);
  gpurenderer = renderer ? (char *)renderer : "";
  if (!gpurenderer.empty())
    return gpurenderer;
  #endif
  gpurenderererror = true;
  return "";
}

long long memory_totalvram() {
  if (videomemory != -1) 
    return videomemory;
  if (videomemoryerror) 
    return -1;
  #if defined(_WIN32)
  IDXGIFactory *pFactory = nullptr;
  if (CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory) == S_OK) {
    IDXGIAdapter *pAdapter = nullptr;
    if (pFactory->EnumAdapters(0, &pAdapter) == S_OK) {
      DXGI_ADAPTER_DESC adapterDesc;
      if (pAdapter->GetDesc(&adapterDesc) == S_OK)
        videomemory = (long long)adapterDesc.DedicatedVideoMemory;
      pAdapter->Release();
    }
    pFactory->Release();
  }
  #elif (defined(__APPLE__) && defined(__MACH__))
  videomemory = strtoll(read_output("ioreg -r -d 1 -w 0 -c \"IOAccelerator\" | grep '\"VRAM,totalMB\"' | uniq | awk -F '= ' '{print $2}'").c_str(), nullptr, 10) * 1024 * 1024;
  #else
  #if defined(CREATE_CONTEXT)
  if (!create_context()) {
    videomemory = -1;
    videomemoryerror = true;
    return -1;
  }
  #endif
  unsigned v = 0;
  PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC queryInteger;
  queryInteger = (PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC)glXGetProcAddressARB((const GLubyte *)"glXQueryCurrentRendererIntegerMESA");
  queryInteger(GLX_RENDERER_VIDEO_MEMORY_MESA, &v);
  videomemory = v * 1024 * 1024;
  #endif
  if (!videomemory)
    videomemory = -1;
  if (videomemory != -1) 
    return videomemory;
  videomemoryerror = true;
  return -1;
}

std::string cpu_processor() {
  if (!cpubrand.empty())
    return cpubrand;
  if (cpubranderror)
    return "";
  #if defined(_WIN32)
  char buf[1024];
  DWORD sz = sizeof(buf);
  if (RegGetValueA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\\", "ProcessorNameString", RRF_RT_REG_SZ, nullptr, &buf, &sz) == ERROR_SUCCESS)
    cpubrand = buf;
  #elif (defined(__APPLE__) && defined(__MACH__))
  char buf[1024];
  std::size_t sz = sizeof(buf);
  if (!sysctlbyname("machdep.cpu.brand_string", &buf, &sz, nullptr, 0))
    cpubrand = buf;
  #elif defined(__linux__)
  cpubrand = read_output("lscpu | grep 'Model name:' | uniq | cut -d' ' -f3- | awk 'NR==1{$1=$1;print}'");
  #endif
  while (!cpubrand.empty() && cpubrand.back() == ' ')
    cpubrand.pop_back();
  if (!cpubrand.empty())
    return cpubrand;
  cpubranderror = true;
  return "";
}

std::string cpu_vendor() {
  if (!cpuvendor.empty())
    return cpuvendor;
  if (cpuvendorerror)
    return "";
  #if defined(_WIN32)
  char buf[1024];
  DWORD sz = sizeof(buf);
  if (RegGetValueA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\\", "VendorIdentifier", RRF_RT_REG_SZ, nullptr, &buf, &sz) == ERROR_SUCCESS)
    cpuvendor = buf;
  #elif (defined(__APPLE__) && defined(__MACH__))
  char buf[1024];
  std::size_t sz = sizeof(buf);
  if (!sysctlbyname("machdep.cpu.vendor", &buf, &sz, nullptr, 0))
    cpuvendor = buf;
  #elif defined(__linux__)
  cpuvendor = read_output("lscpu | grep 'Vendor ID:' | uniq | cut -d' ' -f3- | awk 'NR==1{$1=$1;print}'");
  #endif
  std::string tmp1 = cpu_processor();
  std::transform(tmp1.begin(), tmp1.end(), tmp1.begin(), ::toupper);
  if (cpuvendor.empty() && tmp1.find("APPLE") != std::string::npos)
    cpuvendor = "Apple";
  std::string tmp2 = os_architecture();
  std::transform(tmp2.begin(), tmp2.end(), tmp2.begin(), ::toupper);
  if (cpuvendor.empty() && (tmp2.find("ARM") != std::string::npos || 
    tmp2.find("AARCH64") != std::string::npos))
    cpuvendor = "ARM";
  while (!cpuvendor.empty() && cpuvendor.back() == ' ')
    cpuvendor.pop_back();
  if (!cpuvendor.empty())
    return cpuvendor;
  cpuvendorerror = true;
  return "";
}

int cpu_processor_count() {
  if (numcpus != -1)
    return numcpus;
  if (numcpuserror)
    return -1;
  #if defined(_WIN32)
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  numcpus = sysinfo.dwNumberOfProcessors;
  #elif (defined(__APPLE__) && defined(__MACH__))
  int buf = -1;
  std::size_t sz = sizeof(int);
  if (!sysctlbyname("machdep.cpu.thread_count", &buf, &sz, nullptr, 0))
    numcpus = buf;
  #elif defined(__linux__)
  numcpus = (int)strtol(read_output("lscpu | grep 'CPU(s):' | uniq | cut -d' ' -f4- | awk 'NR==1{$1=$1;print}'").c_str(), nullptr, 10);
  #endif
  if (!numcpus)
    numcpus = -1;
  if (numcpus != -1)
    return numcpus;
  numcpuserror = true;
  return -1;
}

int cpu_core_count() {
  if (numcores != -1)
    return numcores;
  if (numcoreserror)
    return -1;
  #if defined(_WIN32)
  std::string tmp = read_output("wmic cpu get NumberOfCores");
  if (!tmp.empty()) {
    tmp = std::regex_replace(tmp, std::regex("NumberOfCores"), "");
    tmp = std::regex_replace(tmp, std::regex(" "), "");
    tmp = std::regex_replace(tmp, std::regex("\t"), "");
    tmp = std::regex_replace(tmp, std::regex("\r"), "");
    tmp = std::regex_replace(tmp, std::regex("\n"), "");
    numcores = (int)strtol(tmp.c_str(), nullptr, 10);
  }
  #if (defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86))
  /* use x86-specific inline assembly as the fallback; 
  for windows programs run under WINE (no wmic cli) */
  if (!numcores)
    numcores = -1;
  if (numcores != -1)
    return numcores;
  #if defined(_WIN32)
  tmp = cpu_vendor();
  #else
  std::string tmp = cpu_vendor();
  #endif
  std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
  if (tmp.find("INTEL") == std::string::npos && tmp.find("AMD") == std::string::npos) {
    numcoreserror = true;
    return -1;
  }
  class cpuid {
    unsigned regs[4];
  public:
    explicit cpuid(unsigned func_id, unsigned sub_func_id) {
      asm volatile ("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3]) : "a" (func_id), "c" (sub_func_id));
    }
    const unsigned &eax() const {
      return regs[0];
    }
    const unsigned &ebx() const {
      return regs[1];
    }
    const unsigned &ecx() const {
      return regs[2];
    }
    const unsigned &edx() const {
      return regs[3];
    }
  };
  static const unsigned avx_pos = 0x10000000;
  static const unsigned lvl_cores = 0x0000FFFF;
  cpuid cpuid0(0, 0);
  unsigned hfs = cpuid0.eax();
  cpuid cpuid1(1, 0);
  int numsmt = 0;
  bool ishtt = cpuid1.edx() & avx_pos;
  numcpus = cpu_processor_count();
  if (!numcpus) {
    numcoreserror = true;
    return -1;
  }
  if (tmp.find("INTEL") != std::string::npos) {
    if(hfs >= 11) {
      cpuid cpuid4(0x0B, 0);
      numsmt = lvl_cores & cpuid4.ebx();
      numcores = numcpus / numsmt;
    } else {
      if (hfs >= 1) {
        if (hfs >= 4)
          numcores = 1 + ((cpuid(4, 0).eax() >> 26) & 0x3F);
      }
      if (ishtt) {
        if (numcores < 1)
          numcores = 1;
      } else
        numcores = 1;
    }
  } else if (tmp.find("AMD") != std::string::npos) {
    numsmt = 1 + ((cpuid(0x8000001E, 0).ebx() >> 8) & 0xFF);
    if (numcpus > 0 && numsmt > 0)
      numcores = numcpus / numsmt;
    else {
      if (hfs >= 1) {
        if (cpuid(0x80000000, 0).eax() >= 8)
          numcores = 1 + (cpuid(0x80000008, 0).ecx() & 0xFF);
      }
      if (ishtt) {
        if (numcores < 1)
          numcores = 1;
      } else
        numcores = 1;
    }
  }
  #endif
  #elif (defined(__APPLE__) && defined(__MACH__))
  int buf = -1;
  std::size_t sz = sizeof(int);
  if (!sysctlbyname("machdep.cpu.core_count", &buf, &sz, nullptr, 0))
    numcores = buf;
  #elif defined(__linux__)
  int threads_per_core = (int)strtol(read_output("lscpu | grep 'Thread(s) per core:' | uniq | cut -d' ' -f4- | awk 'NR==1{$1=$1;print}'").c_str(), nullptr, 10);
  numcores = (int)(((cpu_processor_count() != -1) ? cpu_processor_count() : 0) / ((threads_per_core) ? threads_per_core : 1));
  #endif
  if (!numcores)
    numcores = -1;
  if (numcores != -1)
    return numcores;
  numcoreserror = true;
  return -1;
}

long long memory_appram() {
  long long appram = -1;
  #if defined(_WIN32)
  PROCESS_MEMORY_COUNTERS pmc;
  if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
    appram = (long long)pmc.WorkingSetSize;
	return appram;
  }
  return -1;
  #elif (defined(__APPLE__) && defined(__MACH__))
  struct proc_taskallinfo info;
  if (proc_pidinfo(getpid(), PROC_PIDTASKALLINFO, 0, &info, sizeof(struct proc_taskallinfo)) > 0) {
    appram = (long long)info.ptinfo.pti_resident_size;
	return appram;
  }
  return -1;
  #elif defined(__linux__)
  auto parse_line = [](char *line) {
    int i = strlen(line);
    const char *p = line;
    while (*p < '0' || *p > '9') p++;
    line[i - 3] = '\0';
    i = strtoll(p, nullptr, 10);
    return (long long)(i * 1024);
  };
  FILE *fp = fopen("/proc/self/status", "r");
  if (fp) {
    char line[128];
    while (fgets(line, 128, fp) != NULL){
      if (strncmp(line, "VmRSS:", 6) == 0){
        appram = parse_line(line);
        break;
      }
    }
    fclose(fp);
  }
  if (appram != -1)
    return appram;
  return -1;
  #endif
}

} // anonymous namespace

static std::string os_info;
EXPORTED_FUNCTION const char *device_os_info(const char *a, const char *b, const char *c, const char *d) {
  os_info.clear();
  if (!strcmp(a, "VERSION")) {
    os_info += std::string(os_kernel_release()) + "\n";
  } else if (!strcmp(a, "RELEASE")) {
    os_info += std::string(os_product_name()) + "\n";
  } else if (!strcmp(a, "ARCHITECTURE")) {
    os_info += std::string(os_architecture()) + "\n";
  } else if (!strcmp(a, "SYSTEM")) {
    os_info += std::string(os_kernel_name()) + "\n";
  }
  if (!strcmp(b, "VERSION")) {
    os_info += std::string(os_kernel_release()) + "\n";
  } else if (!strcmp(b, "RELEASE")) {
    os_info += std::string(os_product_name()) + "\n";
  } else if (!strcmp(b, "ARCHITECTURE")) {
    os_info += std::string(os_architecture()) + "\n";
  } else if (!strcmp(b, "SYSTEM")) {
    os_info += std::string(os_kernel_name()) + "\n";
  }
  if (!strcmp(c, "VERSION")) {
    os_info += std::string(os_kernel_release()) + "\n";
  } else if (!strcmp(c, "RELEASE")) {
    os_info += std::string(os_product_name()) + "\n";
  } else if (!strcmp(c, "ARCHITECTURE")) {
    os_info += std::string(os_architecture()) + "\n";
  } else if (!strcmp(c, "SYSTEM")) {
    os_info += std::string(os_kernel_name()) + "\n";
  }
  if (!strcmp(d, "VERSION")) {
    os_info += std::string(os_kernel_release()) + "\n";
  } else if (!strcmp(d, "RELEASE")) {
    os_info += std::string(os_product_name()) + "\n";
  } else if (!strcmp(d, "ARCHITECTURE")) {
    os_info += std::string(os_architecture()) + "\n";
  } else if (!strcmp(d, "SYSTEM")) {
    os_info += std::string(os_kernel_name()) + "\n";
  }
  while (!os_info.empty() && os_info.back() == '\n') {
    os_info.pop_back();
  }
  return os_info.c_str();
}

static std::string system_info;
EXPORTED_FUNCTION const char *device_system_info(const char *a, const char *b, const char *c, const char *d) {
  system_info.clear();
  if (!strcmp(a, "PROCESSOR")) {
    system_info += std::string(cpu_processor()) + "\n";
  } else if (!strcmp(a, "VENDOR")) {
    system_info += std::string(cpu_vendor()) + "\n";
  } else if (!strcmp(a, "NUMBER_OF_PROCESSORS")) {
    system_info += std::to_string(cpu_processor_count()) + "\n";
  } else if (!strcmp(a, "MEMORY")) {
    system_info += std::to_string(memory_totalram()) + "\n";
  }
  if (!strcmp(b, "PROCESSOR")) {
    system_info += std::string(cpu_processor()) + "\n";
  } else if (!strcmp(b, "VENDOR")) {
    system_info += std::string(cpu_vendor()) + "\n";
  } else if (!strcmp(b, "NUMBER_OF_PROCESSORS")) {
    system_info += std::to_string(cpu_processor_count()) + "\n";
  } else if (!strcmp(b, "MEMORY")) {
    system_info += std::to_string(memory_totalram()) + "\n";
  }
  if (!strcmp(c, "PROCESSOR")) {
    system_info += std::string(cpu_processor()) + "\n";
  } else if (!strcmp(c, "VENDOR")) {
    system_info += std::string(cpu_vendor()) + "\n";
  } else if (!strcmp(c, "NUMBER_OF_PROCESSORS")) {
    system_info += std::to_string(cpu_processor_count()) + "\n";
  } else if (!strcmp(c, "MEMORY")) {
    system_info += std::to_string(memory_totalram()) + "\n";
  }
  if (!strcmp(d, "PROCESSOR")) {
    system_info += std::string(cpu_processor()) + "\n";
  } else if (!strcmp(d, "VENDOR")) {
    system_info += std::string(cpu_vendor()) + "\n";
  } else if (!strcmp(d, "NUMBER_OF_PROCESSORS")) {
    system_info += std::to_string(cpu_processor_count()) + "\n";
  } else if (!strcmp(d, "MEMORY")) {
    system_info += std::to_string(memory_totalram()) + "\n";
  }
  while (!system_info.empty() && system_info.back() == '\n') {
    system_info.pop_back();
  }
  return system_info.c_str();
}

static std::string graphics_info;
EXPORTED_FUNCTION const char *device_graphics_info(const char *a, const char *b, const char *c) {
  graphics_info.clear();
  if (!strcmp(a, "MANUFACTURER")) {
    graphics_info += std::string(gpu_manufacturer()) + "\n";
  } else if (!strcmp(a, "RENDERER")) {
    graphics_info += std::string(gpu_renderer()) + "\n";
  } else if (!strcmp(a, "MEMORY")) {
    graphics_info += std::to_string(memory_totalvram()) + "\n";
  }
  if (!strcmp(b, "MANUFACTURER")) {
    graphics_info += std::string(gpu_manufacturer()) + "\n";
  } else if (!strcmp(b, "RENDERER")) {
    graphics_info += std::string(gpu_renderer()) + "\n";
  } else if (!strcmp(b, "MEMORY")) {
    graphics_info += std::to_string(memory_totalvram()) + "\n";
  }
  if (!strcmp(c, "MANUFACTURER")) {
    graphics_info += std::string(gpu_manufacturer()) + "\n";
  } else if (!strcmp(c, "RENDERER")) {
    graphics_info += std::string(gpu_renderer()) + "\n";
  } else if (!strcmp(c, "MEMORY")) {
    graphics_info += std::to_string(memory_totalvram()) + "\n";
  }
  while (!graphics_info.empty() && graphics_info.back() == '\n') {
    graphics_info.pop_back();
  }
  return graphics_info.c_str();
}

EXPORTED_FUNCTION double device_is_virtual() {
  #if (defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86))
  if (os_product_name().substr(0, 4) == "wine")
    return 1;
  #if (defined(__APPLE__) && defined(__MACH__))
  if (os_architecture() == "x86_64" && 
    read_output("sysctl -in sysctl.proc_translated") == "1")
    return 1;
  #endif
  class cpuid {
    unsigned regs[4];
  public:
    explicit cpuid(unsigned func_id, unsigned sub_func_id) {
      asm volatile ("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3]) : "a" (func_id), "c" (sub_func_id));
    }
    const unsigned &eax() const {
      return regs[0];
    }
    const unsigned &ebx() const {
      return regs[1];
    }
    const unsigned &ecx() const {
      return regs[2];
    }
    const unsigned &edx() const {
      return regs[3];
    }
  };
  if (!(cpuid(1, 0).ecx() & (1 << 31)))
    return 0;
  const int vendorIdLength = 13;
  using VendorIdStr = char[vendorIdLength];
  VendorIdStr hyperVendorId;
  memcpy(hyperVendorId + 0, &cpuid(0x40000000, 0).ebx(), 4);
  memcpy(hyperVendorId + 4, &cpuid(0x40000000, 0).ecx(), 4);
  memcpy(hyperVendorId + 8, &cpuid(0x40000000, 0).edx(), 4);
  hyperVendorId[12] = '\0';
  static const VendorIdStr vendors[] {
    "VBoxVBoxVBox",      // VirtualBox 
    "bhyve bhyve ",      // bhyve
    "KVMKVMKVM\0\0\0",   // KVM
    "TCGTCGTCGTCG",      // QEMU
    #if !defined(_WIN32)
    "Microsoft Hv",      // Microsoft Hyper-V or Windows Virtual PC
    #endif
    "MicrosoftXTA",      // Microsoft x86-to-ARM
    " lrpepyh  vr",      // Parallels
    "VMwareVMware",      // VMware
    "XenVMMXenVMM",      // Xen HVM
    "ACRNACRNACRN",      // Project ACRN
    " QNXQVMBSQG ",      // QNX Hypervisor
    "VirtualApple"       // Apple Rosetta 2
  };
  for (const auto& vendor : vendors) {
    if (!memcmp(vendor, hyperVendorId, vendorIdLength))
      return 1;
  }
  return 0;
  #elif (defined(__APPLE__) && defined(__MACH__))
  return 0;
  #elif defined(__linux__)
  std::string tmp1 = os_architecture();
  std::string tmp2 = read_output("echo $(systemd-detect-virt 2> /dev/null)");
  std::transform(tmp1.begin(), tmp1.end(), tmp1.begin(), ::toupper);
  std::transform(tmp2.begin(), tmp2.end(), tmp2.begin(), ::toupper);
  if (!tmp1.empty() && (tmp1.find("ARM") != std::string::npos || 
    tmp1.find("AARCH64") != std::string::npos)) {
    if (tmp2.empty()) 
      return -1;
    else if (tmp2 == "NONE"))
      return 0;
    return 1;
  }
  #endif
  // non-x86 currently not supported outside of macOS and Linux ...
  return -1;
}

EXPORTED_FUNCTION const char *cpu_number_of_cores() {
  static std::string result;
  result = std::to_string(cpu_core_count());
  return result.c_str();
}

EXPORTED_FUNCTION const char *device_name() {
  static std::string result;
  result = os_device_name();
  return result.c_str();
}

EXPORTED_FUNCTION const char *memory_process() {
  static std::string result;
  result = std::to_string(memory_appram());
  return result.c_str();
}

EXPORTED_FUNCTION const char *memory_used() {
  static std::string result;
  result = std::to_string(memory_usedram());
  return result.c_str();
}

EXPORTED_FUNCTION const char *memory_available() {
  static std::string result;
  result = std::to_string(memory_freeram());
  return result.c_str();
}

EXPORTED_FUNCTION const char *human_readable(const char *nbytes, double roundup) {
  static std::string result;
  result = human_readable(strtoll(nbytes, nullptr, 10), (bool)roundup);
  return result.c_str();
}
