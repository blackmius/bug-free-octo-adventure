#include "Pinger.h"

#include <cstdlib>
#include <iostream>
#include <signal.h>

void onExit(int sigint);

Pinger pinger;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Invalid arguments count.\n";
        return -1;
    }

    pinger = Pinger(argv[1]);

    signal(SIGINT, onExit);
    
    pinger.Ping();
     
    return 0;
}

void onExit(int sigint) {
    pinger.PrintStatistics();
    exit(0);
}