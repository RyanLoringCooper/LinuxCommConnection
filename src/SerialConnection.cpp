#if defined(__linux__) || defined(__linux) || defined(linux)
    #include "Linux/SerialConnection.cpp"
#elif defined(_WIN32)
    #include "Windows/SerialConnection.cpp"
#else
    #error Unsupported os
#endif

SerialConnection::~SerialConnection() {
    terminate();
}
