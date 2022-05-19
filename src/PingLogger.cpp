#include "PingLogger.h"
#include "Exceptions.h"
#include "ErrorCodes.h"

PingLogger::~PingLogger()
{
  //Закрывает лог-файл
  fclose(fd);
}

PingLogger::PingLogger(const char *path)
{
  fd = fopen(path, "a");
  //Проверяем удалось ли утилите открыть или создать лог-файл
  if (!fd)
  {
    throw std::runtime_error("Can't open or create log file.");
  }
};

void PingLogger::log_message(std::string message, bool show)
{
  //Проверяем нужно ли вывести событие в терминале
  if (show)
  {
    //Вывод события в терминале
    printf("%s \n", message.c_str());
  }
  //Запись события в журнал
  fprintf(fd, "%s - %s \n", time_now().c_str(), message.c_str());
}

std::string PingLogger::time_now()
{
  //Структура для получения системного времени
  struct tm *u;
  //Строка для того, чтобы сохранить в ней системное время
  char s1[40] = {0};
  const time_t timer = time(NULL);
  u = localtime(&timer);
  //Преобразования системного времени из структурного типа в строку
  strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
  return s1;
}

std::tuple<PingLogger*, int> PingLogger::CreateLogger(int argc, char **argv)
{
  PingLogger* logger = nullptr;
  try
  {
    if (argc == 3)
    {
      logger = new PingLogger(argv[2]);
    }
    else
    {
      logger = new PingLogger();
    }
  }
  catch(const std::runtime_error& e)
  {
    return std::tuple(nullptr, CANT_OPEN_OR_CREATE_LOG);
  }

  return std::tuple(logger, 0);
}