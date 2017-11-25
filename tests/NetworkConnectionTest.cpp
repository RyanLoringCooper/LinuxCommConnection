#include <iostream>
#include <sys/types.h> 
#include <sys/socket.h>
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

void connectionLoop(NetworkConnection *con, const std::string &conIdentifier) {
    char buff[BUFF_SIZE];
    int avail;
    for(int i = 0; i < 10; i++) {
        con->write(message);
        std::cout << conIdentifier << " sent: " << message << std::endl;
        avail = con->waitForData();
        con->read(buff, avail);
        std::cout << conIdentifier << " recv: " << buff << std::endl;
        if(strcmp(buff, endingMessage.c_str()) == 0) {
            break;
        }
        memset(buff, 0, avail);
    }
    con->write(endingMessage);
    con->terminate();
}

void server(const int &connectionType, const int &port, const int &delayTime) {
    NetworkConnection server = NetworkConnection(port, connectionType, "", delayTime, true);
    server.begin();
    connectionLoop(&server, std::string("server"));
}

void client(const int &connectionType, const int &port, const int &delayTime) {
    NetworkConnection client = NetworkConnection(port, connectionType, "127.0.0.1", delayTime);
    client.begin();
    connectionLoop(&client, std::string("client"));
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
