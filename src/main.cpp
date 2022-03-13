#include "Pinger.h"

#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <functional>

Pinger* pinger = nullptr;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Invalid arguments count.\n";
        return -1;
    }

    pinger = new Pinger(argv[1]);

    signal(SIGINT, [](int sigint) {
        pinger->ShouldEnd = true;
    });

    int result = pinger->Ping();
    if (result != 0) {
        std::cout << "Ping failure.\n";
    }

    delete pinger;

    return 0;
}