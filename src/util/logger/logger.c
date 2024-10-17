#include "logger.h"

#include <windows.h>
#include <stdio.h>

void cosmoriaLogMessage(LogLevel level, const char* message)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    switch (level)
    {
        case COSMORIA_LOG_INFO:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            printf("[INFO]: ");
            break;
        case COSMORIA_LOG_WARNING:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); 
            printf("[WARNING]: ");
            break;
        case COSMORIA_LOG_ERROR:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            printf("[ERROR]: ");
            break;
        default:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            break;
    }

    printf("%s\n", message);

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}