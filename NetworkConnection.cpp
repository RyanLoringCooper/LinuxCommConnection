#if defined(__linux__) || defined(__linux) || defined(linux)
#include "LinuxNetworkConnection.cpp"
#elif defined(_WIN32)
#include "WindowsNetworkConnection.cpp"
#else
#error Unsupported os
#endif

NetworkConnection::NetworkConnection() : CommConnection() {}

NetworkConnection::NetworkConnection(const NetworkConnection &other) {
	if(this == *other) {
		return;
	}
	*this = other;
}

NetworkConnection NetworkConnection::operator=(const NetworkConnection &other) {
    if(this == *other) {
        return this;
    }
    memcpy(buffer, other.buffer, BUFFER_SIZE);
    readIndex = other.readIndex;
    writeIndex = other.writeIndex;
    connected = other.connected;
    interruptRead = other.interruptRead;
    noReads = other.noReads;
    begun = other.begun;
    mSocket = other.mSocket;
    clientSocket = other.clientSocket;
    connAddr = other.connAddr;
    if(begun) {
        begin();
    }
    return *this;
}

NetworkConnection::NetworkConnection(const int &port, const int &connectionType, const char *ipaddr, const bool &noReads) : CommConnection(noReads) {
	this->connectionType = connectionType;
	if(strcmp(ipaddr, "") == 0) {
		if(!setupServer(port)) {
			fprintf(stderr, "Could not setup socket server on port %d.\n", port);
		}
	} else {
		clientSocket = 0;
		if(!setupClient(ipaddr, port)) {
			fprintf(stderr, "Could not setup socket client connection to %s:%d", ipaddr, port);
		}
	}
}

NetworkConnection::~NetworkConnection() {
	exitGracefully();
}

