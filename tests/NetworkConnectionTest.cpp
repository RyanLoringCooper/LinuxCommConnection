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
#define PORT 12347

std::string message("Hello"), endingMessage("exit");

void server(const int &connectionType, const int &delayTime) {
    char recvBuff[BUFF_SIZE];
    memset(&recvBuff[0], 0, BUFF_SIZE);
    NetworkConnection server = NetworkConnection(PORT, connectionType, "", delayTime, true);
    server.begin();
    for(int i = 0; i < 3; i++) {
        std::cout << "server" << i << std::endl;
        server.waitForData();
        server.read(recvBuff, server.available());
        std::cout << "Server got: " << recvBuff << std::endl;
        if(strcmp(recvBuff, endingMessage.c_str()) == 0) {
            server.terminate();
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        memset(&recvBuff[0], 0, BUFF_SIZE);
        server.write(message);
        std::cout << "server wrote\n";
    }
    std::cout << "server quit\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    server.write(endingMessage);
    server.terminate();
}

void client(const int &connectionType, const int &delayTime) {
    char recvBuff[BUFF_SIZE];
    memset(&recvBuff[0], 0, BUFF_SIZE);
    NetworkConnection client = NetworkConnection(PORT, connectionType, "127.0.0.1", delayTime);
    client.begin();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    for(int i = 0; i < 3; i++) {
        std::cout << "client" << i << std::endl;
        client.write(message);
        std::cout << "client wrote\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        client.waitForData();
        client.read(recvBuff, client.available());
        std::cout << "Client got: " << recvBuff << std::endl;
        if(strcmp(recvBuff, endingMessage.c_str()) == 0) {
            client.terminate();
            break;
        }
    }
    std::cout << "client quit\n";
    client.write(endingMessage);
    client.terminate();
}

void runTest(const int &connectionType, const int &delayTime) {
    if(fork() == 0) {
        // child code
        server(connectionType, delayTime);
    } else {
        // parent code
        client(connectionType, delayTime);
        int status;
        waitpid(-1, &status, 0);
    }
}


int main() {
    int connectionTypes[] = {SOCK_STREAM, SOCK_DGRAM};
    int delay[] = {100, 10, 1, 0, -1};

    for(int i = 0; i < 5; i++) {
        if(fork() == 0) {
            int connectionType = connectionTypes[i%2];
            std::cout << "\nTesting " 
                << (SOCK_STREAM == connectionType ? "SOCK_STREAM" : "SOCK_DGRAM") 
                << " with " << delay[i] << "ms timeout on reads" << std::endl;
            runTest(connectionType, delay[i]);
            return 0;
        }
        int status;
        waitpid(-1, &status, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
	return 0;
}
