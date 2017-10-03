#if defined(__linux__) || defined(__linux) || defined(linux)
#include "LinuxSerialConnection.cpp"
#elif defined(_WIN32)
#include "WindowsSerialConnection.cpp"
#else
#error Unsupported os
#endif
