#include "Pinger.h"
#include "PingLogger.h"
#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <functional>
#include <memory>

// Делаем глобальным чтобы иметь доступ в signal. С capture не приводится к обычной функции.
std::unique_ptr<Pinger> pinger;
int main(int argc, char** argv) {
    PingLogger *pingLogger = nullptr;
    // Два, потому что (1)./ping (2)www.google.com
    if (argc != 2 && argc != 3) {
        std::cerr << "Invalid arguments count.\n";
        return -1;
    }

    if (argc == 3)
        pingLogger = new PingLogger(argv[2]);
    else
        pingLogger = new PingLogger();
        
    pingLogger->log_message("Starting application",true);
    // Ловим исключение при создании объекта (неверное имя хоста или ошибка при создании сокета).
    try {
            pinger = std::unique_ptr<Pinger>(new Pinger(argv[1], pingLogger));
    }
    catch(const std::exception& e) {
        pingLogger->log_message(e.what(), true);
        return -1;
    }    
    // Делаем так, что при нажатии Ctrl + C меняется running.
    signal(SIGINT, [](int sigint) {
        pinger->running = false;
    });

    // Пингуем.
    pinger->Ping();

    delete pingLogger;
    
    return 0;
}