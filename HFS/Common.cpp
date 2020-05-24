// self
#include "Common.h"

// windows
#include <Windows.h>

// c/c++
#include <stdio.h>



void ITrace(char* Format, ...)
{
    va_list args = NULL;
    va_start(args, Format);
    size_t length = _vscprintf(Format, args) + 1;
    char* buffer = new char[length];
    _vsnprintf_s(buffer, length, length, Format, args);
    va_end(args);
    OutputDebugStringA(buffer);
    delete[] buffer;
}

