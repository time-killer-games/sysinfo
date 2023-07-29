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
#if (!defined(_WIN32) && (!defined(__APPLE__) && !defined(__MACH__)))
#include <SDL.h>
#include <SDL_opengl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#endif
#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <dxgi.h>
#else
#if defined(__linux__)
#include <sys/sysinfo.h>
#endif
#if ((defined(__APPLE__) && defined(__MACH__)) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__sun))
#include <sys/types.h>
#if (defined(__FreeBSD__) || defined(__DragonFly__))
#include <fcntl.h>
#include <kvm.h>
#elif (defined(__NetBSD__) || defined(__OpenBSD__))
#include <sys/param.h>
#include <sys/swap.h>
#endif
#endif
#if !defined(__sun)
#if !defined(__linux__)
#include <sys/sysctl.h>
#endif
#include <sys/utsname.h>
#else
#include <sys/systeminfo.h>
#include <sys/swap.h>
#endif
#include <unistd.h>
#endif
#if (defined(_WIN32) && defined(_MSC_VER))
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "dxgi.lib")
#endif

#include "pci.ids.hpp"
#include "system.hpp"

namespace ngs::sys {

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

struct hreadable {
  long double size = 0;
  private: friend
  std::ostream& operator<<(std::ostream& os, hreadable hr) {
    int i = 0;
    long double mantissa = hr.size;
    for (; mantissa >= 1024; mantissa /= 1024, i++) { }
    mantissa = std::ceil(mantissa * 100) / 100;
    os << mantissa << " " << "BKMGTPE"[i];
    return !i ? os : os << "B";
  }
};

std::string make_hreadable(long double nbytes) {
  if (nbytes == -1) return "";
  std::stringstream ss;
  ss << hreadable{nbytes};
  return ss.str();
}

/* Define CREATE_CONTEXT in your build scripts or Makefiles if
the calling process hasn't already done this on its own ... */
#if (!defined(_WIN32) && (!defined(__APPLE__) && !defined(__MACH__)))
#if defined(CREATE_CONTEXT)
bool create_context() {
  if (!window) {
    #if (defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__sun))
    setenv("SDL_VIDEODRIVER", "x11", 1);
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    #endif
    if (SDL_Init(SDL_INIT_VIDEO)) return false;
    window = SDL_CreateWindow("", 0, 0, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (!window) return false;
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) return false;
    int err = SDL_GL_MakeCurrent(window, context);
    if (err) return false;
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
#endif

static std::string read_output(std::string cmd) {
  std::string result;
  #if defined(_WIN32)
  bool proceed = true;
  HANDLE stdin_read = nullptr; HANDLE stdin_write = nullptr;
  HANDLE stdout_read = nullptr; HANDLE stdout_write = nullptr;
  SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, true };
  proceed = CreatePipe(&stdin_read, &stdin_write, &sa, 0);
  if (!proceed) return "";
  SetHandleInformation(stdin_write, HANDLE_FLAG_INHERIT, 0);
  proceed = CreatePipe(&stdout_read, &stdout_write, &sa, 0);
  if (!proceed) return "";
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
    HANDLE wait_handles[] = { pi.hProcess, stdout_read };
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
	  result.back() == '\r' || result.back() == '\n')) {
      result.pop_back();
    }
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
  if (vendor_name_by_id.find(identifier) != vendor_name_by_id.end() && !vendor_or_device) return vendor_name_by_id[identifier];
  if (device_name_by_id.find(identifier) != device_name_by_id.end() && vendor_or_device) return device_name_by_id[identifier];
  std::string str(pci_ids, pci_ids + sizeof(pci_ids) / sizeof(pci_ids[0]));
  str = std::regex_replace(str, std::regex("\r"), "");
  std::vector<std::string> vec = string_split(str, '\n');
  for (std::size_t i = 0; i < vec.size(); i++) {
    #if defined(_WIN32)
    message_pump();
    #endif
    if (vec[i].empty() || (!vec[i].empty() && vec[i][0] == '#')) {
      continue;
    }
    std::size_t pos1 = vec[i].find(" ");
    if (pos1 != std::string::npos && vec[i][0] != '\t') {
      std::istringstream converter1(vec[i].substr(0, pos1));
      converter1 >> std::hex >> vendor_id;
      vendor_name_by_id[vendor_id] = vec[i].substr(pos1 + 2);
    }
    if (vendor_name_by_id[vendor_id] != vendor_name_by_id[identifier]) {
      continue;
    }
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

} // anonymous namespace

std::string os_kernel_name() {
  if (!kernelname.empty() || kernelnameerror)
    return kernelname;
  #if !defined(_WIN32)
  #if !defined(__sun)
  struct utsname name;
  if (!uname(&name))
    kernelname = name.sysname;
  #else
  long count = sysinfo(SI_SYSNAME, nullptr, 0);
  if (count > 0) {
    char *buf = (char *)calloc(count, sizeof(char));
    if (buf) {
      if (sysinfo(SI_SYSNAME, buf, count) > 0)
        kernelname = buf;
      free(buf);
    }
  }
  #endif
  #else
  char buf[1024];
  DWORD sz = sizeof(buf);
  if (RegGetValueA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\\", "OS", RRF_RT_REG_SZ, nullptr, &buf, &sz) == ERROR_SUCCESS)
    kernelname = buf;
  #endif
  if (!kernelname.empty())
    return kernelname;
  kernelnameerror = true;
  return kernelname;
}

std::string os_device_name() {
  if (!devicename.empty() || devicenameerror)
    return devicename;
  #if !defined(_WIN32)
  #if !defined(__sun)
  struct utsname name;
  if (!uname(&name))
    devicename = name.nodename;
  #else
  long count = sysinfo(SI_HOSTNAME, nullptr, 0);
  if (count > 0) {
    char *buf = (char *)calloc(count, sizeof(char));
    if (buf) {
      if (sysinfo(SI_HOSTNAME, buf, count) > 0)
        devicename = buf;
      free(buf);
    }
  }
  #endif
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
  return devicename;
}

std::string os_kernel_release() {
  if (!kernelrelease.empty() || kernelreleaseerror)
    return kernelrelease;
  #if !defined(_WIN32)
  #if !defined(__sun)
  struct utsname name;
  if (!uname(&name))
    kernelrelease = name.release;
  #else
  long count = sysinfo(SI_RELEASE, nullptr, 0);
  if (count > 0) {
    char *buf = (char *)calloc(count, sizeof(char));
    if (buf) {
      if (sysinfo(SI_RELEASE, buf, count) > 0)
        kernelrelease = buf;
      free(buf);
    }
  }
  #endif
  #else
  if (!windows_version_number.empty()) {
    kernelrelease = windows_version_number;
	return kernelrelease;
  }
  allocate_windows_version_number_and_product_name();
  kernelrelease = windows_version_number;
  #endif
  if (!kernelrelease.empty())
    return kernelrelease;
  kernelreleaseerror = true;
  return kernelrelease;
}

std::string os_kernel_version() {
  if (!kernelversion.empty() || kernelversionerror)
    return kernelversion;
  #if !defined(_WIN32)
  #if !defined(__sun)
  #if !defined(__DragonFly__)
  struct utsname name;
  if (!uname(&name))
    kernelversion = name.version;
  #else
  /* dragonfly bsd truncates utsname.version;
  get the value from the uname cmd instead */
  static std::string str;
  str = read_output("uname -v");
  return str;
  #endif
  #else
  long count = sysinfo(SI_VERSION, nullptr, 0);
  if (count > 0) {
    char *buf = (char *)calloc(count, sizeof(char));
    if (buf) {
      if (sysinfo(SI_VERSION, buf, count) > 0)
        kernelversion = buf;
      free(buf);
    }
  }
  #endif
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
  return kernelversion;
}

std::string os_product_name() {
  if (!productname.empty() || productnameerror)
    return productname;
  #if defined(_WIN32)
  if (!windows_product_name.empty()) {
    productname = windows_product_name;
	return productname;
  }
  allocate_windows_version_number_and_product_name();
  productname = windows_product_name;
  #elif (defined(__APPLE__) && defined(__MACH__))
  std::string tmp1 = read_output("echo $(sw_vers | grep 'ProductName:' | uniq | awk 'NR==1{$1=$1;print}' && sw_vers | grep 'ProductVersion:' | uniq | awk 'NR==1{$1=$1;print}')");
  if (!tmp1.empty()) {
    tmp1 = std::regex_replace(tmp1, std::regex("ProductName: "), "");
    tmp1 = std::regex_replace(tmp1, std::regex("ProductVersion: "), "");
    std::fstream doc;
    doc.open("/System/Library/CoreServices/Setup Assistant.app/Contents/Resources/en.lproj/OSXSoftwareLicense.rtf", std::ios::in);
    if (doc.is_open()) {
      std::string tmp2;
      while(std::getline(doc, tmp2)) {
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
  std::string tmp = read_output("echo $(lsb_release --id && lsb_release --release && lsb_release --codename) |  tr '\n' ' '");
  if (!tmp.empty()) {
    tmp = std::regex_replace(tmp, std::regex("Distributor ID: "), "");
    tmp = std::regex_replace(tmp, std::regex("Release: "), "");
    tmp = std::regex_replace(tmp, std::regex("Codename: "), "");
    productname = tmp;
  }
  #elif !defined(__sun)
  productname = os_kernel_name() + " " + os_kernel_release();
  #else
  productname = read_output("echo $(lsb_release --id && lsb_release --release && lsb_release --codename) |  tr '\n' ' '");
  #endif
  if (!productname.empty())
    return productname;
  productnameerror = true;
  return productname;
}

std::string os_architecture() {
  if (!architecture.empty() || architectureerror)
    return architecture;
  #if !defined(_WIN32)
  #if !defined(__sun)
  struct utsname name;
  if (!uname(&name))
    architecture = name.machine;
  #else
  long count = sysinfo(SI_ARCHITECTURE_NATIVE, nullptr, 0);
  if (count > 0) {
    char *buf = (char *)calloc(count, sizeof(char));
    if (buf) {
      if (sysinfo(SI_ARCHITECTURE_NATIVE, buf, count) > 0)
        architecture = buf;
      free(buf);
    }
  }
  #endif
  #else
  char buf[1024];
  DWORD sz = sizeof(buf);
  if (RegGetValueA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\\", "PROCESSOR_ARCHITECTURE", RRF_RT_REG_SZ, nullptr, &buf, &sz) == ERROR_SUCCESS)
    architecture = buf;
  #endif
  if (!architecture.empty())
    return architecture;
  architectureerror = true;
  return architecture;
}

std::string memory_totalram(bool human_readable) {
  if (totalram != -1 || totalramerror)
    return human_readable ? make_hreadable(totalram) : std::to_string(totalram);
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex))
    totalram = (long long)statex.ullTotalPhys;
  #elif ((defined(__APPLE__) && defined(__MACH__)) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__))
  int mib[2];
  long long buf = -1;
  mib[0] = CTL_HW;
  #if (defined(__APPLE__) && defined(__MACH__))
  mib[1] = HW_MEMSIZE;
  #elif (defined(__FreeBSD__) || defined(__DragonFly__))
  mib[1] = HW_PHYSMEM;
  #else
  mib[1] = HW_PHYSMEM64;
  #endif
  std::size_t sz = sizeof(long long);
  if (!sysctl(mib, 2, &buf, &sz, nullptr, 0))
    totalram = buf;
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info))
    totalram = (info.totalram * info.mem_unit);
  #elif defined(__sun)
  totalram = strtoll(read_output("prtconf | grep 'Memory size:' | uniq | cut -d' ' -f3- | awk '{print $1 * 1024};'").c_str(), nullptr, 10) * 1024;
  #endif
  if (totalram > 0)
    return human_readable ? make_hreadable(totalram) : std::to_string(totalram);
  totalramerror = true;
  totalram = -1;
  return human_readable ? make_hreadable(totalram) : std::to_string(totalram);
}

std::string memory_freeram(bool human_readable) {
  long long freeram = -1;
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex))
    freeram = (long long)statex.ullAvailPhys;
  #elif ((defined(__APPLE__) && defined(__MACH__)) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__))
  int mib[2];
  long long buf = 0;
  mib[0] = CTL_HW;
  #if ((defined(__APPLE__) && defined(__MACH__)) || defined(__FreeBSD__) || defined(__DragonFly__))
  mib[1] = HW_USERMEM;
  #else
  mib[1] = HW_USERMEM64;
  #endif
  std::size_t sz = sizeof(long long);
  if (!sysctl(mib, 2, &buf, &sz, nullptr, 0))
    freeram = buf;
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info))
    freeram = (info.freeram * info.mem_unit);
  #elif defined(__sun)
  freeram = (sysconf(_SC_AVPHYS_PAGES) * sysconf(_SC_PAGESIZE));
  #endif
  return human_readable ? make_hreadable(freeram) : std::to_string(freeram);
}

std::string memory_usedram(bool human_readable) {
  long long usedram = -1;
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex))
    usedram = (long long)(statex.ullTotalPhys - statex.ullAvailPhys);
  #elif ((defined(__APPLE__) && defined(__MACH__)) ||defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__sun))
  long long total = strtoull(memory_totalram(false).c_str(), nullptr, 10);
  long long avail = strtoull(memory_freeram(false).c_str(), nullptr, 10);
  if (total != -1 && avail != -1)
    usedram = total - avail;
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info))
    usedram = ((info.totalram - info.freeram) * info.mem_unit);
  #endif
  return human_readable ? make_hreadable(usedram) : std::to_string(usedram);
}

std::string memory_totalswap(bool human_readable) {
  long long totalswap = -1;
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex))
    totalswap = (long long)statex.ullTotalPageFile;
  #elif (defined(__APPLE__) && defined(__MACH__))
  struct xsw_usage info;
  std::size_t sz = sizeof(info);
  if (!sysctlbyname("vm.swapusage", &info, &sz, nullptr, 0))
    totalswap = info.xsu_total;
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info))
    totalswap = (info.totalswap * info.mem_unit);
  #elif (defined(__FreeBSD__) || defined(__DragonFly__))
  kvm_t *kvmh = nullptr;
  long page_s = sysconf(_SC_PAGESIZE);
  kvmh = kvm_open(nullptr, "/dev/null", "/dev/null", O_RDONLY, nullptr);
  if (!kvmh) totalswap = -1;
  struct kvm_swap k_swap;
  if (kvm_getswapinfo(kvmh, &k_swap, 1, 0) != -1) {
    kvm_close(kvmh);
    totalswap = k_swap.ksw_total * page_s;
  }
  kvm_close(kvmh);
  #elif (defined(__NetBSD__) || defined(__OpenBSD__))
  long long total = 0;
  long block_s = 0;
  int header_len = 0;
  getbsize(&header_len, &block_s);
  int nswap = swapctl(SWAP_NSWAP, nullptr, 0);
  if (!nswap) totalswap = 0;
  if (nswap > 0) {
    struct swapent *swaps = (struct swapent *)calloc(nswap, sizeof(struct swapent));
    if (swaps) {
      if (swapctl(SWAP_STATS, swaps, nswap) > 0) {
        for (int i = 0; i < nswap; i++) {
          if (swaps[i].se_flags & SWF_ENABLE) {
            total += ((swaps[i].se_nblks / (1024 / block_s)) * 1024);
          }
        }
      }
      free(swaps);
    }
    totalswap = total;
  }
  #elif defined(__sun)
  int i = 0, n = 0;
  long long total = 0;
  long page_s = sysconf(_SC_PAGESIZE);
again:
  int nswap = swapctl(SC_GETNSWP, nullptr);
  if (!nswap) totalswap = 0;
  if (nswap > 0) {
    swaptbl_t *swaps = (swaptbl_t *)malloc((nswap * sizeof(swapent_t)) + sizeof(struct swaptable));
    if (swaps) {
      char *strtab = (char *)malloc((nswap + 1) * 80);
      if (strtab) {
        for (int i = 0; i < (nswap + 1); i++) {
          swaps->swt_ent[i].ste_path = strtab + (i * 80);
        }
        swaps->swt_n = nswap + 1;
        if ((n = swapctl(SC_LIST, swaps)) > 0) {
          if (n > nswap) {
            free(strtab);
            free(swaps);
            goto again;
          }
          for (int i = 0; i < n; i++) {
            total += (swaps->swt_ent[i].ste_pages * page_s);
          }
        }
        free(strtab);
      }
      free(swaps);
    }
    totalswap = total;
  }
  #endif
  return human_readable ? make_hreadable(totalswap) : std::to_string(totalswap);
}

std::string memory_freeswap(bool human_readable) {
  long long freeswap = -1;
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex))
    freeswap = (long long)statex.ullAvailPageFile;
  #elif (defined(__APPLE__) && defined(__MACH__))
  struct xsw_usage info;
  std::size_t sz = sizeof(info);
  if (!sysctlbyname("vm.swapusage", &info, &sz, nullptr, 0))
    freeswap = info.xsu_avail;
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info))
    freeswap = (info.freeswap * info.mem_unit);
  #elif (defined(__FreeBSD__) || defined(__DragonFly__))
  kvm_t *kvmh = nullptr;
  long page_s = sysconf(_SC_PAGESIZE);
  kvmh = kvm_open(nullptr, "/dev/null", "/dev/null", O_RDONLY, nullptr);
  if (!kvmh) freeswap = -1;
  struct kvm_swap k_swap;
  if (kvm_getswapinfo(kvmh, &k_swap, 1, 0) != -1) {
    kvm_close(kvmh);
    freeswap = (k_swap.ksw_total - k_swap.ksw_used) * page_s;
  }
  kvm_close(kvmh);
  #elif (defined(__NetBSD__) || defined(__OpenBSD__))
  long long avail = 0;
  long block_s = 0;
  int header_len = 0;
  getbsize(&header_len, &block_s);
  int nswap = swapctl(SWAP_NSWAP, nullptr, 0);
  if (!nswap) freeswap = 0;
  if (nswap > 0) {
    struct swapent *swaps = (struct swapent *)calloc(nswap, sizeof(struct swapent));
    if (swaps) {
      if (swapctl(SWAP_STATS, swaps, nswap) > 0) {
        for (int i = 0; i < nswap; i++) {
          if (swaps[i].se_flags & SWF_ENABLE) {
            avail += (((swaps[i].se_nblks - swaps[i].se_inuse) / (1024 / block_s)) * 1024);
          }
        }
      }
      free(swaps);
    }
    freeswap = avail;
  }
  #elif defined(__sun)
  int i = 0, n = 0;
  long long avail = 0;
  long page_s = sysconf(_SC_PAGESIZE);
again:
  int nswap = swapctl(SC_GETNSWP, nullptr);
  if (!nswap) freeswap = 0;
  if (nswap > 0) {
    swaptbl_t *swaps = (swaptbl_t *)malloc((nswap * sizeof(swapent_t)) + sizeof(struct swaptable));
    if (swaps) {
      char *strtab = (char *)malloc((nswap + 1) * 80);
      if (strtab) {
        for (int i = 0; i < (nswap + 1); i++) {
          swaps->swt_ent[i].ste_path = strtab + (i * 80);
        }
        swaps->swt_n = nswap + 1;
        if ((n = swapctl(SC_LIST, swaps)) > 0) {
          if (n > nswap) {
            free(strtab);
            free(swaps);
            goto again;
          }
          for (i = 0; i < n; i++) {
            avail += (swaps->swt_ent[i].ste_free * page_s);
          }
        }
        free(strtab);
      }
      free(swaps);
    }
    freeswap = avail;
  }
  #endif
  return human_readable ? make_hreadable(freeswap) : std::to_string(freeswap);
}

std::string memory_usedswap(bool human_readable) {
  long long usedswap = -1;
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex))
    usedswap = (long long)(statex.ullTotalPageFile - statex.ullAvailPageFile);
  #elif (defined(__APPLE__) && defined(__MACH__))
  struct xsw_usage info;
  std::size_t sz = sizeof(info);
  if (!sysctlbyname("vm.swapusage", &info, &sz, nullptr, 0))
    usedswap = info.xsu_used;
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info))
    usedswap = ((info.totalswap - info.freeswap) * info.mem_unit);
  #elif (defined(__FreeBSD__) || defined(__DragonFly__))
  kvm_t *kvmh = nullptr;
  long page_s = sysconf(_SC_PAGESIZE);
  kvmh = kvm_open(nullptr, "/dev/null", "/dev/null", O_RDONLY, nullptr);
  if (!kvmh) usedswap = -1;
  struct kvm_swap k_swap;
  if (kvm_getswapinfo(kvmh, &k_swap, 1, 0) != -1) {
    kvm_close(kvmh);
    usedswap = k_swap.ksw_used * page_s;
  }
  kvm_close(kvmh);
  #elif (defined(__NetBSD__) || defined(__OpenBSD__))
  long long used = 0;
  long block_s = 0;
  int header_len = 0;
  getbsize(&header_len, &block_s);
  int nswap = swapctl(SWAP_NSWAP, nullptr, 0);
  if (!nswap) usedswap = 0;
  if (nswap > 0) {
    struct swapent *swaps = (struct swapent *)calloc(nswap, sizeof(struct swapent));
    if (swaps) {
      if (swapctl(SWAP_STATS, swaps, nswap) > 0) {
        for (int i = 0; i < nswap; i++) {
          if (swaps[i].se_flags & SWF_ENABLE) {
            used += ((swaps[i].se_inuse / (1024 / block_s)) * 1024);
          }
        }
      }
      free(swaps);
    }
    usedswap = used;
  }
  #elif defined(__sun)
  int i = 0, n = 0;
  long long used = 0;
  long page_s = sysconf(_SC_PAGESIZE);
again:
  int nswap = swapctl(SC_GETNSWP, nullptr);
  if (!nswap) usedswap = 0;
  if (nswap > 0) {
    swaptbl_t *swaps = (swaptbl_t *)malloc((nswap * sizeof(swapent_t)) + sizeof(struct swaptable));
    if (swaps) {
      char *strtab = (char *)malloc((nswap + 1) * 80);
      if (strtab) {
        for (int i = 0; i < (nswap + 1); i++) {
          swaps->swt_ent[i].ste_path = strtab + (i * 80);
        }
        swaps->swt_n = nswap + 1;
        if ((n = swapctl(SC_LIST, swaps)) > 0) {
          if (n > nswap) {
            free(strtab);
            free(swaps);
            goto again;
          }
          for (i = 0; i < n; i++) {
            used += ((swaps->swt_ent[i].ste_pages - swaps->swt_ent[i].ste_free) * page_s);
          }
        }
        free(strtab);
      }
      free(swaps);
    }
    usedswap = used;
  }
  #endif
  return human_readable ? make_hreadable(usedswap) : std::to_string(usedswap);
}

std::string gpu_manufacturer() {
  if (!gpuvendor.empty() || gpuvendorerror)
	return gpuvendor;
  #if defined(_WIN32)
  IDXGIFactory *pFactory = nullptr;
  if (CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory) == S_OK) {
    IDXGIAdapter *pAdapter = nullptr;
    if (pFactory->EnumAdapters(0, &pAdapter) == S_OK) {
      DXGI_ADAPTER_DESC adapterDesc;
      if (pAdapter->GetDesc(&adapterDesc) == S_OK) {
        gpuvendor = get_vendor_or_device_name_by_id(adapterDesc.VendorId, 0);
      }
      pAdapter->Release();
    }
    pFactory->Release();
  }
  if (!gpuvendor.empty())
	return gpuvendor;
  #elif (!defined(__APPLE__) && !defined(__MACH__))
  #if defined(CREATE_CONTEXT)
  create_context();
  #endif
  #if defined(__sun)
  unsigned identifier = 0;
  std::istringstream converter(read_output("prtconf |  awk '/display/{p=3} p > 0 {print $0; p--}'| awk -F'pci' 'NR==3{print $0}' | sed 's/.*pci//g' | awk -F' ' '{print $1}' | awk -F',' '{print $1}'"));
  converter >> std::hex >> identifier;
  gpuvendor = get_vendor_or_device_name_by_id(identifier, 0);
  if (!gpuvendor.empty())
	return gpuvendor;
  #endif
  unsigned v = 0;
  PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC queryInteger;
  queryInteger = (PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC)glXGetProcAddressARB((const GLubyte *)"glXQueryCurrentRendererIntegerMESA");
  queryInteger(GLX_RENDERER_VENDOR_ID_MESA, &v);
  gpuvendor = v ? get_vendor_or_device_name_by_id(v, 0) : "";
  if (!gpuvendor.empty())
	return gpuvendor;
  PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC queryString;
  queryString = (PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC)glXGetProcAddressARB((const GLubyte *)"glXQueryCurrentRendererStringMESA");
  gpuvendor = queryString(GLX_RENDERER_VENDOR_ID_MESA);
  if (!gpuvendor.empty())
	return gpuvendor;
  #else
  unsigned identifier = 0;
  std::istringstream converter(read_ouput("ioreg -bls | grep -n2 '    | |   | |   \"model\" = <\"' | awk -F',\"pci' 'NR==5{print $2}' | rev | cut -c 2- | rev | awk -F',' '{print $1}'"));
  converter >> std::hex >> identifier;
  gpuvendor = get_vendor_or_device_name_by_id(identifier, 0);
  if (!gpuvendor.empty()) 
	return gpuvendor;
  gpuvendor = read_ouput("system_profiler SPDisplaysDataType | grep -i 'Vendor: ' | uniq | awk -F 'Vendor: ' 'NR==1{$1=$1;print}' | awk 'NR==1{$1=$1;print}'");
  if (!gpuvendor.empty()) 
	return gpuvendor;
  #endif
  gpuvendorerror = true;
  return "";
}

std::string gpu_renderer() {
  if (!gpurenderer.empty() || gpurenderererror) 
	return gpurenderer;
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
        gpurenderer = get_vendor_or_device_name_by_id(adapterDesc.DeviceId, 1);
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
  #elif (!defined(__APPLE__) && !defined(__MACH__))
  #if defined(CREATE_CONTEXT)
  create_context();
  #endif
  #if defined(__sun)
  unsigned identifier = 0;
  std::istringstream converter(read_output("prtconf | awk '/display/{p=3} p > 0 {print $0; p--}' | awk -F'pci' 'NR==3{print $0}' | sed 's/.*pci//g' | awk -F' ' '{print $1}' | awk -F',' '{print $2}'"));
  converter >> std::hex >> identifier;
  gpurenderer = get_vendor_or_device_name_by_id(identifier, 1);
  if (!gpurenderer.empty())
	return gpurenderer;
  #endif
  unsigned v = 0;
  PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC queryInteger;
  queryInteger = (PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC)glXGetProcAddressARB((const GLubyte *)"glXQueryCurrentRendererIntegerMESA");
  queryInteger(GLX_RENDERER_DEVICE_ID_MESA, &v);
  gpurenderer = v ? get_vendor_or_device_name_by_id(v, 1) : "";
  if (!gpurenderer.empty())
	return gpurenderer;
  PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC queryString;
  queryString = (PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC)glXGetProcAddressARB((const GLubyte *)"glXQueryCurrentRendererStringMESA");
  gpurenderer = queryString(GLX_RENDERER_DEVICE_ID_MESA);
  if (!gpurenderer.empty())
	return gpurenderer;
  #else
  unsigned identifier = 0;
  std::istringstream converter(read_ouput("ioreg -bls | grep -n2 '    | |   | |   \"model\" = <\"' | awk -F',\"pci' 'NR==5{print $2}' | rev | cut -c 2- | rev | awk -F',' '{print $2}'"));
  converter >> std::hex >> identifier;
  gpurenderer = get_vendor_or_device_name_by_id(identifier, 1);
  if (!gpurenderer.empty()) 
	return gpurenderer;
  gpurenderer = read_ouput("system_profiler SPDisplaysDataType | grep -i 'Chipset Model: ' | uniq | awk -F 'Chipset Model: ' 'NR==1{$1=$1;print}' | awk 'NR==1{$1=$1;print}'");
  if (!gpurenderer.empty()) 
	return gpurenderer;
  #endif
  gpurenderererror = true;
  return "";
}

std::string memory_totalvram(bool human_readable) {
  if (videomemory != -1 || videomemoryerror) 
	return human_readable ? make_hreadable(videomemory) : std::to_string(videomemory);
  #if defined(_WIN32)
  IDXGIFactory *pFactory = nullptr;
  if (CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory) == S_OK) {
    IDXGIAdapter *pAdapter = nullptr;
    if (pFactory->EnumAdapters(0, &pAdapter) == S_OK) {
      DXGI_ADAPTER_DESC adapterDesc;
      if (pAdapter->GetDesc(&adapterDesc) == S_OK) {
        videomemory = (long long)adapterDesc.DedicatedVideoMemory;
      }
      pAdapter->Release();
    }
    pFactory->Release();
  }
  #elif (defined(__APPLE__) && defined(__MACH__))
  videomemory = strtoll(read_output("ioreg -r -d 1 -w 0 -c \"IOAccelerator\" | grep '\"VRAM,totalMB\"' | uniq | awk -F '= ' '{print $2}'").c_str(), nullptr, 10) * 1024 * 1024;
  #else
  #if defined(CREATE_CONTEXT)
  create_context();
  #endif
  unsigned v = 0;
  PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC queryInteger;
  queryInteger = (PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC)glXGetProcAddressARB((const GLubyte *)"glXQueryCurrentRendererIntegerMESA");
  queryInteger(GLX_RENDERER_VIDEO_MEMORY_MESA, &v);
  videomemory = v * 1024 * 1024;
  #endif
  if (videomemory > 0) 
	return human_readable ? make_hreadable(videomemory) : std::to_string(videomemory);
  videomemoryerror = true;
  videomemory = -1;
  return human_readable ? make_hreadable(videomemory) : std::to_string(videomemory);
}

std::string cpu_vendor() {
  if (!cpuvendor.empty() || cpuvendorerror)
    return cpuvendor;
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
  #elif defined(__FreeBSD__)
  cpuvendor = read_output("dmesg | awk '/CPU: /{p++;if(p==0){next}}p' | awk -F'Origin=\"' 'NR==2{print $0}' | sed 's/.*Origin=\"//g' | awk -F' ' '{print $1}' | awk -F',' '{print $1}' | awk '{print substr($0, 1, length($0) - 1)}'");
  #elif defined(__DragonFly__)
  cpuvendor = read_output("dmesg | awk '/CPU: /{p++;if(p==0){next}}p' | awk -F'Origin = \"' 'NR==2{print $0}' | sed 's/.*Origin = \"//g' | awk -F' ' '{print $1}' | awk -F',' '{print $1}' | awk '{print substr($0, 1, length($0) - 1)}'");
  #elif defined(__NetBSD__)
  cpuvendor = read_output("cat /proc/cpuinfo | grep 'vendor_id' | awk 'NR==1{print $3}'");
  #elif defined(__OpenBSD__)
  cpuvendor = read_output("sysctl -n machdep.cpuvendor");
  #elif defined(__sun)
  cpuvendor = read_output("psrinfo -v -p | awk 'NR==2{print substr($2, 2)}'");
  #endif
  if (!cpuvendor.empty())
    return cpuvendor;
  cpuvendorerror = true;
  return "";
}

std::string cpu_processor() {
  if (!cpubrand.empty() && cpubranderror)
    return cpubrand;
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
  #elif (defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__))
  int mib[2];
  char buf[1024];
  mib[0] = CTL_HW;
  mib[1] = HW_MODEL;
  std::size_t sz = sizeof(buf);
  if (!sysctl(mib, 2, buf, &sz, nullptr, 0))
    cpubrand = buf;
  #elif defined(__sun)
  cpubrand = read_output("psrinfo -v -p | awk 'NR==3{print}' | awk 'NR==1{$1=$1;print}''");
  #endif
  if (!cpubrand.empty())
    return cpubrand;
  cpubranderror = true;
  return "";
}

int cpu_core_count() {
  if (numcores != -1 || numcoreserror)
    return numcores;
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
  #elif (defined(__APPLE__) && defined(__MACH__))
  int buf = -1;
  std::size_t sz = sizeof(int);
  if (!sysctlbyname("machdep.cpu.core_count", &buf, &sz, nullptr, 0))
    numcores = buf;
  #elif defined(__linux__)
  int threads_per_core = (int)strtol(read_output("lscpu | grep 'Thread(s) per core:' | uniq | cut -d' ' -f4- | awk 'NR==1{$1=$1;print}'").c_str(), nullptr, 10);
  numcores = (int)(cpu_processor_count() / ((threads_per_core) ? threads_per_core : 1));
  #elif defined(__FreeBSD__)
  /* number_of_thread_groups will return zero if threads are not grouped at all; this means the number of cores equals the number of cpus */
  int number_of_thread_groups = (int)strtol(read_ouput("sysctl -n kern.sched.topology_spec | grep -c 'THREAD group'").c_str(), nullptr, 10);
  numcores = number_of_thread_groups ? number_of_thread_groups : cpu_processor_count();
  #elif defined(__DragonFly__)
  int threads_per_core = (int)strtol(read_output("dmesg | grep 'threads_per_core: ' | awk '{print substr($6, 0, length($6) - 1)}'").c_str(), nullptr, 10);
  numcores = (int)(cpu_processor_count() / ((threads_per_core) ? threads_per_core : 1));
  #elif (defined(_WIN32) || defined(__NetBSD__) || defined(__OpenBSD__))
  #if defined(_WIN32)
  /* use x86-specific inline assembly as the fallback; 
  for windows programs run under WINE (no wmic cli) */
  if (numcores > 0) {
    return numcores;
  }
  #endif
  /* netbsd and openbsd have no exposed api for getting the number of cores; 
  we use x86-specific inline assembly for intel-and-amd-based cpus when it's
  possible to do so. If the current platform is not i386/amd64 this fails */
  cpuvendor = cpu_vendor();
  if (os_architecture() != "i386" || os_architecture() != "amd64" ||
    cpuvendor != "GenuineIntel" || cpuvendor != "AuthenticAMD" || cpuvendor != "AMDisbetter!")
    goto failed;
  class CPUID {
    unsigned regs[4];
    public:
    explicit CPUID(unsigned funcId, unsigned subFuncId) {
      asm volatile ("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3]) : "a" (funcId), "c" (subFuncId));
    }
    const unsigned &EAX() const { return regs[0]; }
    const unsigned &EBX() const { return regs[1]; }
    const unsigned &ECX() const { return regs[2]; }
    const unsigned &EDX() const { return regs[3]; }
  };
  static const unsigned AVX_POS = 0x10000000;
  static const unsigned LVL_CORES = 0x0000FFFF;
  CPUID cpuID0(0, 0);
  unsigned HFS = cpuID0.EAX();
  CPUID cpuID1(1, 0);
  int mNumSMT = 0;
  bool mIsHTT = cpuID1.EDX() & AVX_POS;
  numcpus = cpu_processor_count();
  if (cpuvendor == "GenuineIntel") {
    if(HFS >= 11) {
      CPUID cpuID4(0x0B, 0);
      mNumSMT = LVL_CORES & cpuID4.EBX();
      numcores = numcpus / mNumSMT;
    } else {
      if (HFS >= 1) {
        if (HFS >= 4) {
          numcores = 1 + (CPUID(4, 0).EAX() >> 26) & 0x3F;
        }
      }
      if (mIsHTT) {
        if (numcores < 1) {
          numcores = 1;
        }
      } else {
        numcores = 1;
      }
    }
  } else if (cpuvendor == "AuthenticAMD" || cpuvendor == "AMDisbetter!") {
    mNumSMT = 1 + ((CPUID(0x8000001e, 0).EBX() >> 8) & 0xFF);
    if (numcpus > 0 && mNumSMT > 0) {
      numcores = numcpus / mNumSMT;
    } else {
      if (HFS >= 1) {
        if (CPUID(0x80000000, 0).EAX() >= 8) {
          numcores = 1 + (CPUID(0x80000008, 0).ECX() & 0xFF);
        }
      }
      if (mIsHTT) {
        if (numcores < 1) {
          numcores = 1;
        }
      } else {
        numcores = 1;
      }
    }
  }
failed:
  #elif defined(__sun)
  numcores = (int)strtol(read_output("echo `expr $(kstat cpu_info | grep 'pkg_core_id' | uniq | wc -l | awk '{print $1}') / $(psrinfo -p)`").c_str(), nullptr, 10);
  #endif
  if (numcores > 0)
    return numcores;
  numcoreserror = true;
  numcores = -1;
  return numcores;
}

int cpu_processor_count() {
  if (numcpus != -1 || numcpuserror)
    return numcpus;
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
  #elif (defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__))
  int mib[2];
  mib[0] = CTL_HW;
  mib[1] = HW_NCPU;
  int buf = -1;
  std::size_t sz = sizeof(int);
  if (!sysctl(mib, 2, &buf, &sz, nullptr, 0))
    numcpus = buf;
  #elif defined(__sun)
  numcpus = (int)strtol(read_output("psrinfo -v -p | grep 'The physical processor has ' | awk '{print $5}'").c_str(), nullptr, 10);
  #endif
  if (numcpus > 0)
    return numcpus;
  numcpuserror = true;
  numcpus = -1;
  return numcpus;
}

} // namespace ngs::sys
