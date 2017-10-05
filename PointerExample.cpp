#include <thread>
#include <chrono>
#include "NetworkConnection.h"
#define PORT 12334

NetworkConnection *con0, *con1;

void startCon0() {
    con0 = new NetworkConnection(PORT, SOCK_STREAM);
    printf("fack!\n");
    con0->begin();
    printf("fack!\n");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    con0->write("dirt", 5);
    con0->terminate();
}

void startCon1() {
    con1 = new NetworkConnection(PORT, SOCK_STREAM, "127.0.0.1");
    printf("fack!\n");
    con1->begin();
    printf("fack!\n");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    con1->write("dirt", 5);
    con1->terminate();
}

int main(int argc, char *argv[]) {
    std::thread t0(startCon0);
    std::thread t1(startCon1);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    t0.join();
    t1.join();
    printf("wack!\n");
    delete con0;
    printf("wack!\n");
    delete con1;
    printf("wack!\n");
    return 0;
}
