#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// c/c++
#include <stdexcept>


class WinAPIException : public std::runtime_error
{
public:
    WinAPIException(const char* Message, int ErrorCode);

    int GetErrorCode() const;

private:
    int m_ErrorCode;
};


class WSAException : public WinAPIException
{
public:
    WSAException(const char* Message, int ErrorCode);
};