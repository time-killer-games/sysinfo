#if defined(_WIN32) && defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <cmath>
#if defined(_WIN32)
#define EXPORTED_FUNCTION extern "C" __declspec(dllexport)
#include <winsock2.h>
#include <windows.h>
#include <intrin.h>
#include <GL/glew.h>
#include <GL/glext.h>
#include <GL/gl.h>
#else
#define EXPORTED_FUNCTION extern "C" __attribute__((visibility("default")))
#if (defined(__APPLE__) && defined(__MACH__))
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#if defined(__linux__)
#include <sys/sysinfo.h>
#include <cpuid.h>
#endif
#include <sys/utsname.h>
#endif
#if defined(_WIN32)
#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#endif
#endif

struct HumanReadable {
  double size{};
  private: friend
  std::ostream& operator<<(std::ostream& os, HumanReadable hr) {
    int i{};
    double mantissa = hr.size;
    for (; mantissa >= 1024; mantissa /= 1024, ++i) { }
    mantissa = std::ceil(mantissa * 10) / 10;
    os << mantissa << " " << "BKMGTPE"[i];
    return i == 0 ? os : os << "B";
  }
};

#if defined(_WIN32)
bool initglew = false;
HGLRC create_opengl_context(HDC *hdc) {
  if (!initglew) {
    glewInit();
    initglew = true;
  }
  GLuint PixelFormat;
  glewExperimental = GL_TRUE;
  static PIXELFORMATDESCRIPTOR pfd = {
  sizeof(PIXELFORMATDESCRIPTOR), 1, 
  PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 
  PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                
  32, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0 };
  HDC hDC = GetDC(GetDesktopWindow());
  PixelFormat = ChoosePixelFormat(hDC, &pfd);
  SetPixelFormat(hDC, PixelFormat, &pfd);
  HGLRC hRC = wglCreateContext(hDC);
  wglMakeCurrent(hDC, hRC);
  *hdc = hDC;
  return hRC;
}
#endif

EXPORTED_FUNCTION const char *sysname() {
  #if !defined(_WIN32)
  const char *result = nullptr;
  struct utsname name;
  if (!uname(&name))
    result = name.sysname;
  static std::string str;
  str = result ? result : "";
  return str.c_str();
  #else
  return "Windows_NT";
  #endif
}

EXPORTED_FUNCTION const char *nodename() {
  #if !defined(_WIN32)
  const char *result = nullptr;
  struct utsname name;
  if (!uname(&name))
    result = name.nodename;
  static std::string str;
  str = result ? result : "";
  return str.c_str();
  #else
  const char *result = nullptr;
  char buf[1024];
  WSADATA data;
  WORD wVersionRequested;
  wVersionRequested = MAKEWORD(2, 2);
  if (!WSAStartup(wVersionRequested, &data)) {
    if (!gethostname(buf, sizeof(buf))) {
      result = buf;    
    }
    WSACleanup();
  }
  static std::string str;
  str = result ? result : "";
  return str.c_str();
  #endif
}

EXPORTED_FUNCTION const char *release() {
  #if !defined(_WIN32)
  const char *result = nullptr;
  struct utsname name;
  if (!uname(&name))
    result = name.release;
  static std::string str;
  str = result ? result : "";
  return str.c_str();
  #else
  auto GetOSMajorVersionNumber = []() {
    const char *result = nullptr;
    char buf[10];
    int val = 0;  
    DWORD sz = sizeof(val);
    if (RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "CurrentMajorVersionNumber", RRF_RT_REG_DWORD, nullptr, &val, &sz) == ERROR_SUCCESS) {
      if (sprintf(buf, "%d", val) != -1) {
        result = buf;
      }
    }
    static std::string str;
    str = result ? result : "";
    return str.c_str();
  };
  auto GetOSMinorVersionNumber = []() {
    const char *result = nullptr;
    char buf[10];
    int val = 0; 
    DWORD sz = sizeof(val);
    if (RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "CurrentMinorVersionNumber", RRF_RT_REG_DWORD, nullptr, &val, &sz) == ERROR_SUCCESS) {
      if (sprintf(buf, "%d", val) != -1) {
        result = buf;
      }
    }
    static std::string str;
    str = result ? result : "";
    return str.c_str();
  };
  auto GetOSBuildNumber = []() {
    const char *result = nullptr;
    char buf[255]; 
    DWORD sz = sizeof(buf);
    if (RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "CurrentBuildNumber", RRF_RT_REG_SZ, nullptr, &buf, &sz) == ERROR_SUCCESS) {
      result = buf;
    }
    static std::string str;
    str = result ? result : "";
    return str.c_str();
  };
  auto GetOSRevisionNumber = []() {
    char *result = nullptr;
    char buf[10];
    int val = 0; 
    DWORD sz = sizeof(val);
    if (RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "UBR", RRF_RT_REG_DWORD, nullptr, &val, &sz) == ERROR_SUCCESS) {
      if (sprintf(buf, "%d", val) != -1) {
        result = buf;
      }
    }
    static std::string str;
    str = strlen(result) ? result : "";
    return str.c_str();
  };
  static const char *result = nullptr;
  char buf[1024];
  if (strcmp(GetOSMajorVersionNumber(), "") && strcmp(GetOSMinorVersionNumber(), "") && strcmp(GetOSBuildNumber(), "") && strcmp(GetOSRevisionNumber(), "")) {
    if (sprintf(buf, "%s.%s.%s.%s", GetOSMajorVersionNumber(), GetOSMinorVersionNumber(), GetOSBuildNumber(), GetOSRevisionNumber()) != -1) {
      result = buf;
    }
  }
  static std::string str;
  str = result ? result : "";
  return str.c_str();
  #endif
}

EXPORTED_FUNCTION const char *version() {
  #if !defined(_WIN32)
  const char *result = nullptr;
  struct utsname name;
  if (!uname(&name))
    result = name.version;
  static std::string str;
  str = result ? result : "";
  return str.c_str();
  #else
  const char *result = nullptr;
  char buf[2048];
  if (sprintf(buf, "%s%s%s", "Microsoft Windows [Version ", release(), "]") != -1) {
    result = buf;
  }
  static std::string str;
  str = result ? result : "";
  return str.c_str();
  #endif
}

EXPORTED_FUNCTION const char *machine() {
  #if !defined(_WIN32)
  const char *result = nullptr;
  struct utsname name;
  if (!uname(&name))
    result = name.machine;
  static std::string str;
  str = result ? result : "";
  return str.c_str();
  #else
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
    return "AMD64";  
  } else if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM) {
    return "ARM";
  } else if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64) {
    return "ARM64"; 
  } else if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
    return "IA64"; 
  } else if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
    return "x86"; 
  }
  return "";
  #endif
}

EXPORTED_FUNCTION double totalram() {
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex)) {
    return (double)statex.ullTotalPhys;
  }
  return -1;
  #elif (defined(__APPLE__) && defined(__MACH__))
  int mib[2];
  long long physical_memory;
  mib[0] = CTL_HW;
  mib[1] = HW_MEMSIZE;
  std::size_t len = sizeof(long long);
  if (!sysctl(mib, 2, &physical_memory, &len, nullptr, 0)) {
    return physical_memory;
  }
  return -1;
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info)) {
    return info.totalram;
  }
  return -1;
  #endif
}

EXPORTED_FUNCTION double availram() {
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex)) {
    return (double)statex.ullAvailPhys;
  }
  return -1;
  #elif (defined(__APPLE__) && defined(__MACH__))
  vm_size_t page_size;
  mach_port_t mach_port;
  mach_msg_type_number_t count;
  vm_statistics64_data_t vm_stats;
  mach_port = mach_host_self();
  count = sizeof(vm_stats) / sizeof(natural_t);
  if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
    KERN_SUCCESS == host_statistics64(mach_port, HOST_VM_INFO,
    (host_info64_t)&vm_stats, &count)) {
    long long free_memory = (long long)vm_stats.free_count * (long long)page_size;
    long long used_memory = ((long long)vm_stats.active_count +
    (long long)vm_stats.inactive_count +
    (long long)vm_stats.wire_count) *  (long long)page_size;
    return free_memory;
  }
  return -1;
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info)) {
    return info.freeram;
  }
  return -1; 
  #endif
}

EXPORTED_FUNCTION double usedram() {
  #if defined(_WIN32)
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex)) {
    return (double)(statex.ullTotalPhys - statex.ullAvailPhys);
  }
  return -1;
  #elif (defined(__APPLE__) && defined(__MACH__))
  vm_size_t page_size;
  mach_port_t mach_port;
  mach_msg_type_number_t count;
  vm_statistics64_data_t vm_stats;
  mach_port = mach_host_self();
  count = sizeof(vm_stats) / sizeof(natural_t);
  if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
    KERN_SUCCESS == host_statistics64(mach_port, HOST_VM_INFO,
    (host_info64_t)&vm_stats, &count)) {
    long long free_memory = (long long)vm_stats.free_count * (long long)page_size;
    long long used_memory = ((long long)vm_stats.active_count +
    (long long)vm_stats.inactive_count +
    (long long)vm_stats.wire_count) *  (long long)page_size;
    return used_memory;
  }
  return -1;
  #elif defined(__linux__)
  struct sysinfo info;
  if (!sysinfo(&info)) {
    return info.totalram - info.freeram;
  }
  return -1; 
  #endif
}

EXPORTED_FUNCTION const char *cpuvendor() {
  #if defined(_WIN32)
  const char *result = nullptr;
  int regs[4];
  char vendor[13];
  __cpuid(regs, 0);
  memcpy(vendor, &regs[1], 4);
  memcpy(vendor + 4, &regs[3], 4);
  memcpy(vendor + 8, &regs[2], 4);
  vendor[12] = '\0';
  std::string untrimmed;
  untrimmed = vendor ? vendor : "";
  std::size_t pos = untrimmed.find_first_not_of(" ");
  if (pos != std::string::npos) {
    static std::string str;
    str = untrimmed.substr(pos);
    return str.c_str();
  }
  return "";
  #elif (defined(__APPLE__) && defined(__MACH__))
  char buf[1024];
  const char *result = nullptr;
  std::size_t len = sizeof(buf);
  if (!sysctlbyname("machdep.cpu.vendor", &buf, &len, nullptr, 0)) {
    result = buf;
  }
  static std::string str;
  str = result ? result : "";
  return str.c_str();
  #elif defined(__linux__)
  char buf[1024];
  const char *result = nullptr;
  FILE *fp = popen("cat /proc/cpuinfo | grep 'vendor_id' | uniq | awk -F ' ' '{print $3}'", "r");
  if (fp) {
    if (fgets(buf, sizeof(buf), fp)) {
      buf[strlen(buf) - 1] = '\0';
      result = buf;
    }
    pclose(fp);
    static std::string str;
    str = result ? result : "";
    return str.c_str();
  }
  return "";
  #endif
}

EXPORTED_FUNCTION const char *cpubrand() {
  #if defined(_WIN32)
  const char *result = nullptr;
  int CPUInfo[4];
  unsigned nExIds, i = 0;
  char CPUBrandString[0x40];
  __cpuid(CPUInfo, 0x80000000);
  nExIds = CPUInfo[0];
  for (i = 0x80000000; i <= nExIds; i++) {
    __cpuid(CPUInfo, i);
    if  (i == 0x80000002) {
      memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
    } else if  (i == 0x80000003) {
      memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
    } else if  (i == 0x80000004) {
      memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
    }
  }
  std::string untrimmed;
  result = CPUBrandString;
  untrimmed = result ? result : "";
  std::size_t pos = untrimmed.find_first_not_of(" ");
  if (pos != std::string::npos) {
    static std::string str;
    str = untrimmed.substr(pos);
    return str.c_str();
  }
  return "";
  #elif (defined(__APPLE__) && defined(__MACH__))
  const char *result = nullptr;
  char buf[1024];
  std::size_t len = sizeof(buf);
  if (!sysctlbyname("machdep.cpu.brand_string", &buf, &len, nullptr, 0)) {
    result = buf;
  }
  static std::string str;
  str = result ? result : "";
  return str.c_str();
  #elif defined(__linux__)
  const char *result = nullptr;
  char CPUBrandString[0x40];
  unsigned CPUInfo[4] = { 0, 0, 0, 0 };
  __cpuid(0x80000000, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
  unsigned nExIds = CPUInfo[0];
  memset(CPUBrandString, 0, sizeof(CPUBrandString));
  for (unsigned i = 0x80000000; i <= nExIds; i++) {
    __cpuid(i, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
    if (i == 0x80000002) {
      memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
    } else if (i == 0x80000003) {
      memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
    } else if (i == 0x80000004) {
      memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
    }
  }
  std::string untrimmed;
  result = CPUBrandString;
  untrimmed = result ? result : "";
  std::size_t pos = untrimmed.find_first_not_of(" ");
  if (pos != std::string::npos) {
    static std::string str;
    str = untrimmed.substr(pos);
    return str.c_str();
  }
  return "";
  #endif
}

EXPORTED_FUNCTION double numcpus() {
  double result = std::thread::hardware_concurrency();
  return result ? result : -1;
}

EXPORTED_FUNCTION const char *gpuvendor() {
  #if defined(_WIN32)
  HDC hdc = nullptr;
  HGLRC context = create_opengl_context(&hdc);
  #endif
  const char *result = (char *)glGetString(GL_VENDOR);
  static std::string str;
  str = result ? result : "";
  #if defined(_WIN32)
  if (context) wglDeleteContext(context);
  if (hdc) DeleteDC(hdc);
  #endif
  return str.c_str();
}

EXPORTED_FUNCTION const char *gpurenderer() {
  #if defined(_WIN32)
  HDC hdc = nullptr;
  HGLRC context = create_opengl_context(&hdc);
  #endif
  const char *result = (char *)glGetString(GL_RENDERER);
  static std::string str;
  str = result ? result : "";
  #if defined(_WIN32)
  if (context) wglDeleteContext(context);
  if (hdc) DeleteDC(hdc);
  #endif
  return str.c_str();
}

EXPORTED_FUNCTION const char *gpuversion() {
  #if defined(_WIN32)
  HDC hdc = nullptr;
  HGLRC context = create_opengl_context(&hdc);
  #endif
  const char *result = (char *)glGetString(GL_VERSION);
  static std::string str;
  str = result ? result : "";
  #if defined(_WIN32)
  if (context) wglDeleteContext(context);
  if (hdc) DeleteDC(hdc);
  #endif
  return str.c_str();
}

EXPORTED_FUNCTION const char *gpushadervers() {
  #if defined(_WIN32)
  HDC hdc = nullptr;
  HGLRC context = create_opengl_context(&hdc);
  #endif
  const char *result = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
  static std::string str;
  str = result ? result : "";
  #if defined(_WIN32)
  if (context) wglDeleteContext(context);
  if (hdc) DeleteDC(hdc);
  #endif
  return str.c_str();
}

EXPORTED_FUNCTION const char *hreadable(double nbytes) {
  if (nbytes == -1) return "";
  std::stringstream ss;
  static std::string str;
  ss << HumanReadable{nbytes};
  str = ss.str();
  return str.c_str();
}

int main() {
  std::cout << "nodename(): " << nodename() << "\n" << 
  "version(): " << version() << "\n" <<  
  "release(): " << release() << "\n" <<  
  "machine(): " << machine() << "\n" <<  
  "sysname(): " << sysname() << "\n" <<  
  "pubrand(): " << cpubrand() << "\n" <<  
  "cpuvendor(): " << cpuvendor() << "\n" <<  
  "numcpus(): " << numcpus() << "\n" <<  
  "totalram(): " << hreadable(totalram()) << "\n" << 
  "gpuvendor(): " << gpuvendor() << "\n" << 
  "gpurenderer(): " << gpurenderer() << "\n" << 
  "gpuversion(): " << gpuversion() << "\n" << 
  "gpushadervers(): " << gpushadervers() << "\n" << 
  "usedram(): " << hreadable(usedram()) << "\n" << 
  "availram(): " << hreadable(availram()) << "\n";
  return 0;
}
