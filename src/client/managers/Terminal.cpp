#include "Terminal.h"

#include <cstdio>
#include <cstdarg>

void printAt(int x, int y, const char* format, ...) {
    // 1. Skocz do pozycji (y - wiersz, x - kolumna)
    printf("\033[%d;%dH", y, x);

    // 2. Obsłuż resztę argumentów jak w printf
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    // 3. Wymuś wypisanie (ważne w pętlach!)
    fflush(stdout);
}
