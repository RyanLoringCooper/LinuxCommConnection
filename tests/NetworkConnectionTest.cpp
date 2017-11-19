#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <thread>
#include <chrono>
#include <string>
#include "../src/NetworkConnection.h"

#define BUFF_SIZE 256
#define PORT 12345

std::string message("Hello"), endingMessage("exit");

void runTest(const int &connectionType, const int &delayTime) {
    if(fork() == 0) {
        // child code
        char recvBuff[BUFF_SIZE];
        memset(&recvBuff[0], 0, BUFF_SIZE);
        NetworkConnection server = NetworkConnection(PORT, connectionType, "", delayTime);
        server.begin();
        for(int i = 0; i < 3; i++) {
            server.waitForData();
            server.read(recvBuff, server.available());
            std::cout << "Server got: " << recvBuff << std::endl;
            if(strcmp(recvBuff, endingMessage.c_str()) == 0) {
                break;
            }
            memset(&recvBuff[0], 0, BUFF_SIZE);
            server.write(message);
        }
        server.write(endingMessage);
        server.terminate();
    } else {
        // parent code
        char recvBuff[BUFF_SIZE];
        memset(&recvBuff[0], 0, BUFF_SIZE);
        NetworkConnection client = NetworkConnection(PORT, connectionType, "127.0.0.1", delayTime);
        client.begin();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        for(int i = 0; i < 3; i++) {
            client.write(message);
            client.waitForData();
            client.read(recvBuff, client.available());
            std::cout << "Client got: " << recvBuff << std::endl;
            if(strcmp(recvBuff, endingMessage.c_str()) == 0) {
                break;
            }
        }
        client.write(endingMessage);
        client.terminate();
        int status;
        waitpid(-1, &status, 0);
    }
}

int main() {
    std::cout << "Testing UDP with blocking reads" << std::endl;
	runTest(SOCK_DGRAM, -1);
    std::cout << "\nTesting TCP with blocking reads" << std::endl;
	runTest(SOCK_STREAM, -1);
    std::cout << "\nTesting UDP with 100 ms timeout on reads" << std::endl;
	runTest(SOCK_DGRAM, 100);
    std::cout << "\nTesting TCP with 100 ms timeout on reads" << std::endl;
	runTest(SOCK_STREAM, 100);
    std::cout << "\nTesting UDP with 10 ms timeout on reads" << std::endl;
	runTest(SOCK_DGRAM, 10);
    std::cout << "\nTesting TCP with 10 ms timeout on reads" << std::endl;
	runTest(SOCK_STREAM, 10);
    std::cout << "\nTesting UDP with 0 ms timeout on reads" << std::endl;
	runTest(SOCK_DGRAM, 0);
    std::cout << "\nTesting TCP with 0 ms timeout on reads" << std::endl;
	runTest(SOCK_STREAM, 0);
	return 0;
}
