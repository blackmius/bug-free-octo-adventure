#include "PingLogger.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <time.h>

PingLogger::PingLogger(const char* path){
    fd = fopen (path,"a");
};

void PingLogger::log_message(std::string message, bool show){
    if (show) {
        printf("%s \n", message.c_str());
    }
    fprintf(fd, "%s - %s \n", time_now().c_str(), message.c_str());
}

std::string PingLogger::time_now(){
    struct tm *u;
    char s1[40] = { 0 };
    const time_t timer = time(NULL);
    u = localtime(&timer);
    strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
    return s1;
}
