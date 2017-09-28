#include "SerialConnection.h"

// protected:
void SerialConnection::failedRead() {
	printf("Failed to read from serial port with error: %d", errno);
}

int SerialConnection::getData(char *buff, const int &buffSize) {
	if(!connected)
		return -1; 

	return 0;
}

void SerialConnection::exitGracefully() {
	if (connected && ser > 0) {
		connected = false;
		close(ser);
	}
}

// public:
SerialConnection::SerialConnection() : CommConnection() {
	connected = false;
}

SerialConnection::SerialConnection(char *portName) : CommConnection() {
	connected = false;
}

SerialConnection::~SerialConnection() {
	exitGracefully();	
}

bool SerialConnection::write(char *buff, const int &buffSize) {
	if(!connected) 
		return false;
}