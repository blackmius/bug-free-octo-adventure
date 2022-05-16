#pragma once

#include <stdexcept>

class BeforeLogError : public std::runtime_error {
public:

    BeforeLogError(const std::string& message) : std::runtime_error(message) {}

};

class AfterLogError : public std::runtime_error {
public:

    AfterLogError(const std::string& message) : std::runtime_error(message) {}

};