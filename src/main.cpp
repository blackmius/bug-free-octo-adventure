#include "Pinger.h"

#include <cstdlib>
#include <iostream>
#include <signal.h>

void onExit(int sigint);

Pinger* pinger = nullptr;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Invalid arguments count.\n";
        return -1;
    }

    pinger = new Pinger(argv[1]);

    signal(SIGINT, onExit);
    
    pinger->Ping();
    
    delete pinger;

    return 0;
}

void onExit(int sigint) {
    pinger->PrintStatistics();
    
    delete pinger;

    exit(0);
}