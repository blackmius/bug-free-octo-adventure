#include "PingLogger.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <time.h>

PingLogger::~PingLogger(){
  //Закрывает лог-файл
  fclose(fd);
}

PingLogger::PingLogger(const char* path){
  fd = fopen (path,"a");
  //Проверяем удалось ли утилите открыть или создать лог-файл
  if (!fd) {
        throw std::runtime_error("Ivalid log-file name.");
  }
};

void PingLogger::log_message(std::string message, bool show){
  //Проверяем нужно ли вывести событие в терминале
  if(show){
    //Вывод события в терминале
    printf("%s \n", message.c_str());
  }
  //Запись события в журнал
  fprintf(fd, "%s - %s \n", time_now().c_str(), message.c_str());
}

std::string PingLogger::time_now(){
  //Структура для получения системного времени
  struct tm *u;
  //Строка для того, чтобы сохранить в ней системное время
  char s1[40] = { 0 };
  const time_t timer = time(NULL);
  u = localtime(&timer);
  //Преобразования системного времени из структурного типа в строку
  strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
  return s1;
}
