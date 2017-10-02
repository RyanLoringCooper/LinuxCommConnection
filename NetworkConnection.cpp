#if defined(__linux__) || defined(__linux) || defined(linux)
#include "LinuxNetworkConnection.cpp"
#elif defined(_WIN32)
#include "WindowsNetworkConnection.cpp"
#else
#error Unsupported os
#endif
