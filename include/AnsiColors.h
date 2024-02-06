#ifndef AnsiColors
#define AnsiColors

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define FORMAT_RESET "\e[m"
#define BOLD "\e[1m"
#define DIM "\e[2m"
#define ITALIC "\e[3m"
#define NOITALIC "\e[23m"
#define UNDERLINE "\e[4m"
#define NOUNDERLINE "\e[24m"
#define RED "\e[31m"
#define GREEN "\e[32m"
#define YELLOW "\e[33m"
#define BLUE "\e[34m"
#define MAGENTA "\e[35m"
#define CYAN "\e[36m"
void Colors(void);

#endif