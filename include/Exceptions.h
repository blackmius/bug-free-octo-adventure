#pragma once

#include <stdexcept> // std::runtime_error

// Класс исключения, используемый при ошибке создания сокета. 
class SocketCreationError : public std::exception
{
public:

    SocketCreationError() : std::exception() {}

};