#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// c/c++
#include <stdint.h>


class StatPrinter
{
public:
    StatPrinter();

    bool Print(size_t NumClients);

private:
    uint64_t m_LastPrinted;
};
