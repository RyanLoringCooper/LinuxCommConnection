#if defined(__linux__) || defined(__linux) || defined(linux)
#include "LinuxNetworkConnection.cpp"
#elif defined(_WIN32)
#include "WindowsNetworkConnection.cpp"
#else
#error Unsupported os
#endif

NetworkConnection::NetworkConnection(const int &port, const int &connectionType, const char *ipaddr, const int &blockingTime, const bool &noReads) : CommConnection(blockingTime, noReads) {
	this->connectionType = connectionType;
	if(strcmp(ipaddr, "") == 0) {
        server = true;
		if(!setupServer(port)) {
			fprintf(stderr, "Could not setup socket server on port %d.\n", port);
		}
	} else {
        server = false;
		if(!setupClient(ipaddr, port)) {
			fprintf(stderr, "Could not setup socket client connection to %s:%d", ipaddr, port);
		}
	}
}

NetworkConnection::~NetworkConnection() {
	terminate();
}

