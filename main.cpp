#include <iostream>
#include <math.h>
#include <thread>
#include "namedChannelsClient.cpp"
#include "namedChannelsServer.cpp"

using namespace std;

int main(int argc, char* argv[]) {
    std::thread serverThread(server);
    client();
    serverThread.join();
    return 0;
}


