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

const std::string message("Hello"), endingMessage("exit");
const std::string helpText("Usage:\n\tNetworkConnectionTest [-h] [-d <delay_time>] -s <socket_type> -c <connection_type> -p <port>\n\n\t" 
        "-h shows this help text\n\t"
        "-d denotes that the next argument is the time to delay between internal reads.\n\t"
        "<delay_time> = the time to delay between internal reads. -1 means a read is a blocking. 0 means no delay between reads. Times greater than zero are in milliseconds. Defaults to -1.\n\t"
        "-s denotes that the next argument is the socket type.\n\t"
        "<socket_type> = The type of socket to setup for this program. Possible options are client or server.\n\t"
        "-c denotes that the next argument is the connection type.\n\t"
        "<connection_type> = The connection type to tests. Possible options are tcp or udp.\n\t"
        "-p denotes that the next argument is the port number\n\t"
        "<port> = A number from 1-65535 which is the port to connect on\n"
        );

void server(const int &connectionType, const int &port, const int &delayTime) {
    char recvBuff[BUFF_SIZE];
    memset(&recvBuff[0], 0, BUFF_SIZE);
    NetworkConnection server = NetworkConnection(port, connectionType, "", delayTime, true);
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

void client(const int &connectionType, const int &port, const int &delayTime) {
    char recvBuff[BUFF_SIZE];
    memset(&recvBuff[0], 0, BUFF_SIZE);
    NetworkConnection client = NetworkConnection(port, connectionType, "127.0.0.1", delayTime);
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
        server(connectionType, 12345, delayTime);
    } else {
        // parent code
        client(connectionType, 12345, delayTime);
        int status;
        waitpid(-1, &status, 0);
    }
}


int testRun() {
    int connectionTypes[] = {SOCK_STREAM, SOCK_DGRAM};
    int delay[] = {100, 10, 1, 0, -1};

    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 2; j++) {
            if(fork() == 0) {
                int connectionType = connectionTypes[j];
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
    }
	return 0;
}

int main(int argc, char *argv[]) {
    if(argc == 1) {
        std::cerr << "Improper usage.\n";
        std::cout << helpText << std::endl;
        return 1;
    } 
    int connectionType = -1, port = -1, delayTime = -1;
    std::string socketType("NULL");
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-h") == 0) {
            break;
        } else if(strcmp(argv[i], "-s") == 0) {
            if(i == argc-1) {
                std::cerr << "Improper usage. Socket type must come after -s flag.\n";
                break;
            } else if(strcmp(argv[i+1], "server") != 0 && strcmp(argv[i+1], "client") != 0) {
                std::cerr << "Improper usage. Incorrect socket type option. It must be either server or client.\n";
                break;
            } else {
                socketType = std::string(argv[++i]);
            }
        } else if(strcmp(argv[i], "-c") == 0) {
            if(i == argc-1) {
                std::cerr << "Improper usage. Conneciton type must come after -c flag.\n";
                break;
            } else if(strcmp(argv[i+1], "tcp") != 0 && strcmp(argv[i+1], "udp") != 0) {
                std::cerr << "Improper usage. Incorrect socket type option. It must be either tcp or udp.\n";
                break;
            } else {
                i++;
                if(strcmp(argv[i], "tcp") == 0) {
                    connectionType = SOCK_STREAM;
                } else {
                    connectionType = SOCK_DGRAM;
                }
            }
        } else if(strcmp(argv[i], "-p") == 0) {
            if(i == argc-1) {
                std::cerr << "Improper usage. A port number must come after -p flag.\n";
                break;
            } 
            port = atoi(argv[++i]);
            if(port < 1 || port > 65535) {
                std::cerr << "Improper usage. Incorrect port option. It must be a number in decimal from 1-65535\n";
                port = -1;
                break;
            }
        } else if(strcmp(argv[i], "-d") == 0) {
            if(i == argc-1) {
                continue;
            } 
            delayTime = atoi(argv[i++]);
            if(delayTime < -1) {
                std::cerr << "delay_time must be greater than -2. Defaulting to -1.\n";
                delayTime = -1;
            }
        }
    }
    if(connectionType == -1 || port == -1 || socketType == "NULL") {
        std::cout << helpText << std::endl;
    } else {
        if(socketType == "server") {
            server(connectionType, port, delayTime);
        } else {
            client(connectionType, port, delayTime);
        }
    }
    return 0;
}
