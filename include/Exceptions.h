// Файл пользовательский исключений.

#pragma once

#include <stdexcept> // std::runtime_error

// Класс исключения, используемый при ошибке создания сокета.
class SocketCreationError : public std::exception
{
public:
    SocketCreationError() : std::exception() {}
};

// Класс исключения, используемый при ошибке записи данных в журнал.
class LogWriteError : public std::exception
{
public:
    LogWriteError() : std::exception() {}
};