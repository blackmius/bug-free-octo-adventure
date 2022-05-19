#pragma once

#include <string> // std::string
#include <netdb.h> // hostent, gethostbyname(), sockaddr_in
#include <memory>

#include "PingLogger.h"
#include "ICMPHeader.h"

// Тип реализующий функционал ping'a.
class Pinger
{

private:
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

    // реккурентное стандартное отклонение
    // http://www.scalaformachinelearning.com/2015/10/recursive-mean-and-standard-deviation.html
    double zhmih;

    // Логгер.
    PingLogger *pingLogger;

    /**
     * @brief Функция для расчета чек-суммы пакета.
     * @param buf Указатель на начало пакета.
     * @param size Размер пакета.
     * @return uint16_t Чек-сумма.
     * 
     * Подробнее
     * https://datatracker.ietf.org/doc/html/rfc1071
     * 
     */
    uint16_t calculateChecksum(uint16_t* buf, int32_t size);

    // Добавить комментарии.

    /**
     * @brief Выполняет отправку пакета.  
     * 
     * @param lastPacketSendTime указатель на переменную, хранящую время отправки последнего пакета.
     */
    int sendPackage(int64_t *lastPacketSendTime);

    /**
     * @brief Выполняет получение пакета.
     * 
     * @param buffer указатель на буфер получаемого пакета.
     * @param bufferSize размер буфера получаемого пакета.
     * @return true если получение успешно.
     * @return false если получение провалено.
     */
    int recvPackage(unsigned char *buffer, size_t bufferSize);

    /**
     * @brief Обновляет статистику и выводит ифнормацию о последнем полученном пакете.
     * 
     * @param buffer указатель на буфер полученого пакета. 
     * @param bufferSize размер буфера полученного пакета.
     */
    void updateStatistic(unsigned char *buffer, size_t bufferSize);

    /**
     * @brief Выводит финальную статистику работы пинга.
     */
    void outputStatistic();

public:

    // Определяет, должен ли продолжать работу Pinger.
    bool running = true;

public:

    /**
     * @brief Создает новый экземпляр Pinger
     * 
     * @param host IPv4 или домен хоста.
     * @param pingLogger Указатель на логгер.
     */
    explicit Pinger(const char* host, PingLogger *pingLogger);

    /**
     * @brief Выполняет функционал ping'a.
     */
    int Ping();

    /**
     * @brief Выполняет валидацию переданных аргументов. 
     * 
     * @param argc количество переданных аргументов. 
     */
    static int ValidateArgs(int argc);

    static std::tuple<Pinger*, int> CreatePinger(const char* host, PingLogger *pingLogger);
};