#include "Pinger.h"

#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <functional>
#include <memory>

// Делаем глобальным чтобы иметь доступ в signal. С capture не приводится к обычной функции
std::unique_ptr<Pinger> pinger;

int main(int argc, char** argv) {
    
    // Два, потому что (1)./ping (2)www.google.com
    if (argc != 2) {
        std::cerr << "Invalid arguments count.\n";
        return -1;
    }

    // Ловим исключение при создании объекта (неверное имя хоста или ошибка при создании сокета).
    try {
        pinger = std::unique_ptr<Pinger>(new Pinger(argv[1]));
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return -1;
    }

    // Делаем так, что при нажатии Ctrl + C меняется running.
    signal(SIGINT, [](int sigint) {
        pinger->running = false;
    });

    // Пингуем.
    pinger->Ping();
    
    return 0;
}