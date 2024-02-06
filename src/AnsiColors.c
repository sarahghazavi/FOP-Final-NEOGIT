#include "AnsiColors.h"
void Colors(void)
{
    #ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        SetConsoleMode(hOut, dwMode | 0x0004);
    #else
        printf("\e[?25h\e[0m");
    #endif
}