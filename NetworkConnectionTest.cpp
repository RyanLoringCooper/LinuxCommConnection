#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include "NetworkConnection.h"

#define BUFF_SIZE 256
#define PORT 12345

void runTest(const int &connectionType) {
	if(fork() == 0) {
		// child code
		if(fork() == 0) {
			// child code
			char recvBuff[BUFF_SIZE];
			memset(&recvBuff[0], 0, BUFF_SIZE);
			NetworkConnection server = NetworkConnection(PORT, connectionType);
			server.begin();
			for(int i = 0; i < 3; i++) {
				while(!server.available());
				server.read(recvBuff, server.available());
				std::cout << "Server got: " << recvBuff << std::endl;
				memset(&recvBuff[0], 0, BUFF_SIZE);
				server.write("Hello", 6);
			}
			server.terminate();
		} else {
			// parent code
			char recvBuff[BUFF_SIZE];
			memset(&recvBuff[0], 0, BUFF_SIZE);
			NetworkConnection client = NetworkConnection(PORT, connectionType, "127.0.0.1");
			client.begin();
			for(int i = 0; i < 3; i++) {
				client.write("Hello", 6);
				while(!client.available());
				client.read(recvBuff, client.available());
				std::cout << "Client got: " << recvBuff << std::endl;
			}
			client.terminate();
			int status;
			waitpid(-1, &status, 0);
		}
	} else {
		// parent code
		int status;
		waitpid(-1, &status, 0);
	}
	std::cout << "First test done\n";
}

int main() {
	runTest(SOCK_DGRAM);
	runTest(SOCK_STREAM);
	return 0;
}