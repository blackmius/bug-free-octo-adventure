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
    
    // Два или три, потому что (1)./ping (2)www.google.com (3)logfile.log.
    if (argc != 2 && argc != 3) {
        std::cerr << "Invalid arguments count.\n";
        return -1;
    }
    // Ловим исключение при создании объекта журнала (неверное имя файла или недостаточно места для создания файла).
    try {
        // Если аргумента три, берем название лог-файла из третьего аргумента.
        if (argc == 3)
            pingLogger = new PingLogger(argv[2]);
        // Иначе он получает стандартное название.
        else
            pingLogger = new PingLogger();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return -1;
    }
    
    // Записываем в журнал событие о начале работы приложения    
    pingLogger->log_message("Starting application",true);

    // Ловим исключение при создании объекта (неверное имя хоста или ошибка при создании сокета).
    try {
        pinger = std::unique_ptr<Pinger>(new Pinger(argv[1], pingLogger));
    }
    catch(const std::exception& e) {
        pingLogger->log_message(e.what(), true);
        return -1;
    }

    // Обрабатываем Ctrl + С для завершения программы.
    signal(SIGINT, [](int sigint) {
        pinger->running = false;
    });

    // Пингуем.
    pinger->Ping();

    delete pingLogger;
    
    return 0;
}