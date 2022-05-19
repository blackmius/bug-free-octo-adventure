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
        case 0:
        {
            auto [ip, getIpCode] = Pinger::GetIp(argv[1]);
            switch (getIpCode)
            {
                case 0:
                {
                    // Создаем логгер. В идеале не забыть удалить.
                    auto [pingLogger, createLoggerCode] = PingLogger::CreateLogger(argc, argv);
                    switch (createLoggerCode)
                    {
                        case 0:
                        {
                            // Записываем в журнал событие о начале работы приложения    
                            pingLogger->log_message("Starting application");

                            // Создаем объект Pinger.
                            auto [p, createPingerCode]= Pinger::CreatePinger(argv[1], ip, pingLogger);
                            switch (createPingerCode)
                            {
                                case 0:
                                {
                                    pinger = std::unique_ptr<Pinger>(p);

                                    // Обрабатываем Ctrl + С для завершения программы.
                                    signal(SIGINT, [](int sigint)
                                    {
                                        pinger->running = false;
                                    });
                                    //test
                                    // Пингуем.
                                    int pingCode = pinger->Ping();
                                    switch (pingCode)
                                    {
                                        case 0:
                                            return 0;
                                        case SEND_ERROR:
                                        case RECV_ERROR:
                                            Pinger::EndWithError(pingCode, pingLogger);
                                    }
                                }
                                break;
                                case SOCKET_CREATION_ERROR:
                                    Pinger::EndWithError(createPingerCode, pingLogger);
                            }
                        }
                        break;
                        case CANT_OPEN_OR_CREATE_LOG:
                            Pinger::EndWithError(createLoggerCode);
                    }
                }
                break;
                case HOST_ERROR:
                    Pinger::EndWithError(getIpCode);
            }
        }
        break;
        case INVALID_ARGUMENTS_COUNT:
            Pinger::EndWithError(validateArgsCode);
    }
    return 0;
}