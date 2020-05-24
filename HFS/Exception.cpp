// self
#include "Exception.h"

// project
#include "Common.h"

// windows
#include <Windows.h>


WinAPIException::WinAPIException(const char* message, int errorCode)
    : std::runtime_error(message)
    , m_ErrorCode(errorCode)
{
    ITRACE("");

}

int WinAPIException::GetErrorCode() const
{
    ITRACE("");

    return m_ErrorCode;
}

WSAException::WSAException(const char* message, int errorCode)
    : WinAPIException(message, errorCode)
{
    ITRACE("");

}
