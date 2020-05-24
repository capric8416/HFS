// self
#include "Stat.h"

// project
#include "Common.h"

// windows
#include <Windows.h>



StatPrinter::StatPrinter()
    : m_LastPrinted(0)
{
    ITRACE("");
}


bool StatPrinter::Print(size_t numClients)
{
    ITRACE("");

    static const uint64_t DELAY = 1000;

    const uint64_t ts = GetTickCount64();
    if (m_LastPrinted + DELAY < ts)
    {
        ITRACE("active clients: %ull", numClients);
        m_LastPrinted = ts;
        return true;
    }
    return false;
}
