#ifndef COSMORIA_LOGGER_H
#define COSMORIA_LOGGER_H


typedef enum LogLevel {
    COSMORIA_LOG_INFO,
    COSMORIA_LOG_WARNING,
    COSMORIA_LOG_ERROR
} LogLevel;


void cosmoriaLogMessage(LogLevel level, const char* message);

#endif