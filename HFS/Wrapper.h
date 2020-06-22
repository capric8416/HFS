#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// windows
#include <winsock2.h>

// c/++
#include <stdint.h>


class SocketWrapper
{
public:
    SocketWrapper();
    SocketWrapper(SOCKET socket);
    SocketWrapper(SocketWrapper&& other);
    virtual ~SocketWrapper();

    SOCKET Get() const;

    explicit operator bool() const;

private:
    SocketWrapper(const SocketWrapper& other) = delete;
    SocketWrapper& operator=(const SocketWrapper&) = delete;

protected:
    SOCKET m_Socket;
};


class ListeningSocketWrapper : public SocketWrapper
{
public:
    ListeningSocketWrapper(unsigned short port, uint16_t* SetPort);
};


class WSAEventWrapper
{
public:
    WSAEventWrapper();

    virtual ~WSAEventWrapper();

    const WSAEVENT& Get() const;

private:
    WSAEventWrapper(const WSAEventWrapper& other) = delete;
    WSAEventWrapper& operator=(const WSAEventWrapper&) = delete;

private:
    WSAEVENT m_Event;
};

