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

int runTest(const int &connectionType, const int &delayTime) {
    int pid = fork();
    if(pid == 0) {
        // child code
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
        return pid;
    } else {
        // parent code
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
        int status;
        waitpid(-1, &status, 0);
        return pid;
    }
}


int main() {
    int pid;

    std::cout << "\nTesting UDP with 100 ms timeout on reads" << std::endl;
	pid = runTest(SOCK_DGRAM, 100);
    if(pid == 0) {
        return 0;
    }

    std::cout << "\nTesting TCP with 100 ms timeout on reads" << std::endl;
	pid = runTest(SOCK_STREAM, 100);
    if(pid == 0) {
        return 0;
    }

    std::cout << "\nTesting UDP with 10 ms timeout on reads" << std::endl;
	pid = runTest(SOCK_DGRAM, 10);
    if(pid == 0) {
        return 0;
    }

    std::cout << "\nTesting TCP with 10 ms timeout on reads" << std::endl;
	pid = runTest(SOCK_STREAM, 10);
    if(pid == 0) {
        return 0;
    }

    std::cout << "\nTesting UDP with 0 ms timeout on reads" << std::endl;
	pid = runTest(SOCK_DGRAM, 0);
    if(pid == 0) {
        return 0;
    }

    std::cout << "\nTesting TCP with 0 ms timeout on reads" << std::endl;
	pid = runTest(SOCK_STREAM, 0);
    if(pid == 0) {
        return 0;
    }

    std::cout << "Testing UDP with blocking reads" << std::endl;
	pid = runTest(SOCK_DGRAM, -1);
    if(pid == 0) {
        std::cout << "Child exiting" << std::endl;
        return 0;
    }

    std::cout << "\nTesting TCP with blocking reads" << std::endl;
	pid = runTest(SOCK_STREAM, -1);
    if(pid == 0) {
        return 0;
    }
	return 0;
}
