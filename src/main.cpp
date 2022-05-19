#include "Pinger.h" // Pinger
#include "PingLogger.h" // PingLogger
#include "ErrorCodes.h"

#include <signal.h> // signal(), SIGINT
#include <memory> // std::unique_ptr

// Делаем глобальным чтобы иметь доступ в signal. С capture не приводится к обычной функции.
std::unique_ptr<Pinger> pinger;

// Тело программы
int main(int argc, char** argv)
{
    // Обрабатываем исключения, возникшие в результате работы программы.
    // Пока логер не создан, можем выводить ошибки в стандатный поток вывода ошибок.

    // Валидируем количество аргументов.
    int validateArgsCode = Pinger::ValidateArgs(argc);
    switch (validateArgsCode) {
        case 0: {
            // Создаем логгер. В идеале не забыть удалить.
            auto [pingLogger, createLoggerCode] = PingLogger::CreateLogger(argc, argv);
            switch (createLoggerCode) {
                case 0: {
                    // Записываем в журнал событие о начале работы приложения    
                    pingLogger->log_message("Starting application");

                    // Создаем объект Pinger.
                    auto [p, createPingerCode]= Pinger::CreatePinger(argv[1], pingLogger);
                    switch (createPingerCode) {
                        case 0: {
                            pinger = std::unique_ptr<Pinger>(p);

                            // Обрабатываем Ctrl + С для завершения программы.
                            signal(SIGINT, [](int sigint)
                            {
                                pinger->running = false;
                            });

                            // Пингуем.
                            int pingCode = pinger->Ping();
                            switch (pingCode) {
                                case 0:
                                    return 0;
                                case SEND_ERROR:
                                    pingLogger->log_message("Error when sending packet.", true);
                                    return SEND_ERROR;
                                case RECV_ERROR:
                                    pingLogger->log_message("Error when receiving packet.", true);
                                    return RECV_ERROR;
                            }
                        }
                        break;
                        case HOST_ERROR:
                            pingLogger->log_message("Host detection error.", true);
                            return HOST_ERROR;
                        case SOCKET_CREATION_ERROR:
                            pingLogger->log_message("Socket creation failure.", true);
                            return SOCKET_CREATION_ERROR;
                    }
                }
                break;
                case CANT_OPEN_OR_CREATE_LOG:
                    std::cerr << "Can't open or create log file.\n";
                    return CANT_OPEN_OR_CREATE_LOG;
            }
        }
        break;
        case INVALID_ARGUMENTS_COUNT:
            std::cerr << "Invalid arguments count given. Example: sudo ./ping www.google.com log.log(optional).\n";
            return INVALID_ARGUMENTS_COUNT;
    }
    return 0;
}