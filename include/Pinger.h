#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include "PingLogger.h"


#include "ICMPHeader.h"

// Тип реализующий функционал ping'a.
class Pinger {

private:

    static const int BUFSIZE = 64;

    // Информация о хосте предоставляемая пользователем.
    std::string host;
    std::string ip;
    
    // Частота отправки пакетов (в секундах).
    int64_t timeout = 1;

    // Дескриптор сокета.
    int socket;
    // Информация, используемая при создании сокета.
    sockaddr_in sockAddr;

    // Статистика.
    int sendPacketsCount;
    int recvPacketsCount;

    double minPingTime;
    double maxPingTime;
    double avgPingTime;
    double preAvgPingTime;
    double mdev;

    // Логгер.
    PingLogger *pingLogger;

    /**
     * @brief Функция для расчета чек-суммы пакета.
     * @param buf Указатель на начало пакета.
     * @param size Размер пакета.
     * @return uint16_t - Чек-сумма.
     */
    uint16_t calculateChecksum(uint16_t* buf, int32_t size);

public:
    // Определяет, должен ли продолжать работу Pinger.
    bool running = true;

public:

    /**
     * @brief Construct a new Pinger object
     * 
     * @param host IPv4 или домен хоста.
     * @param pingLogger Указатель на логгер.
     */
    explicit Pinger(const char* host, PingLogger *pingLogger);

    /**
     * @brief Выполняет функционал ping'a.
     */
    void Ping();
};