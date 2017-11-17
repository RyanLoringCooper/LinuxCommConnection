#if defined(__linux__) || defined(__linux) || defined(linux)
#include "Linux/LinuxSerialConnection.cpp"
#elif defined(_WIN32)
#include "Windows/WindowsSerialConnection.cpp"
#else
#error Unsupported os
#endif

SerialConnection::~SerialConnection() {
    terminate();
}
