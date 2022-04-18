#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <time.h>
class PingLogger {
private:
  FILE * fd;
public:
  PingLogger(const char* path = "log.txt");
  std::string time_now();
  void log_message(std::string message, bool show = false);
};