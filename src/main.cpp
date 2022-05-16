#include "Pinger.h" // Pinger
#include "PingLogger.h" // PingLogger
#include "Exceptions.h" // BeforeLogError, AfterLogError

#include <signal.h> // signal(), SIGINT
#include <memory> // std::unique_ptr

// Делаем глобальным чтобы иметь доступ в signal. С capture не приводится к обычной функции.
std::unique_ptr<Pinger> pinger;

// Тело программы
int main(int argc, char** argv)
{
    PingLogger *pingLogger = nullptr;
    
    // Обрабатываем исключения, возникшие в результате работы программы.
    // Пока логер не создан, можем выводить ошибки в стандатный поток вывода ошибок.
    try {

        // Валидируем количество аргументов.
        Pinger::ValidateArgs(argc);

        // Создаем логгер. В идеале не забыть удалить.
        pingLogger = PingLogger::CreateLogger(argc, argv);

        // Записываем в журнал событие о начале работы приложения    
        pingLogger->log_message("Starting application");

        // Создаем объект Pinger.
        pinger = std::unique_ptr<Pinger>(new Pinger(argv[1], pingLogger));
    }
    catch(const BeforeLogError& e)
    {
        std::cerr << e.what() << "\n";
        return -1;
    }
    catch(const AfterLogError& e)
    {
        pingLogger->log_message(e.what(), true);
        return -1;
    }

    // Обрабатываем Ctrl + С для завершения программы.
    signal(SIGINT, [](int sigint)
    {
        pinger->running = false;
    });

    // Пингуем.
    pinger->Ping();

    delete pingLogger;

    return 0;
}