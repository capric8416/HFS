// self
#include "Exception.h"

// project
#include "Common.h"

// windows
#include <Windows.h>


WinAPIException::WinAPIException(const char* Message, int ErrorCode)
    : std::runtime_error(Message)
    , m_ErrorCode(ErrorCode)
{
    ITRACE("code:%d, msg: %s", ErrorCode, Message);
}

int WinAPIException::GetErrorCode() const
{
    return m_ErrorCode;
}

WSAException::WSAException(const char* Message, int ErrorCode)
    : WinAPIException(Message, ErrorCode)
{
    ITRACE("code:%d, msg: %s", ErrorCode, Message);
}
