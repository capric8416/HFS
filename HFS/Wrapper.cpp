// self
#include "Wrapper.h"

// project
#include "Common.h"
#include "Exception.h"

// windows
#include <winsock2.h>
#include <Windows.h>



SocketWrapper::SocketWrapper()
    : m_Socket(INVALID_SOCKET)
{
    ITRACE("");
}


SocketWrapper::SocketWrapper(SOCKET socket)
    : m_Socket(socket)
{
    ITRACE("");
}


SocketWrapper::SocketWrapper(SocketWrapper&& other)
    : m_Socket(INVALID_SOCKET)
{
    ITRACE("");

    std::swap(m_Socket, other.m_Socket);
}


SocketWrapper::~SocketWrapper()
{
    ITRACE("");

    if (m_Socket != INVALID_SOCKET) {
        if (closesocket(m_Socket) != 0) {
            ITRACE("Closing socket failed: %0x", WSAGetLastError());
        }
    }
}


SOCKET SocketWrapper::Get() const
{
    ITRACE("");

    return m_Socket;
}


SocketWrapper::operator bool() const
{
    ITRACE("");

    return (m_Socket != INVALID_SOCKET);
}


ListeningSocketWrapper::ListeningSocketWrapper(unsigned short port, uint16_t* SetPort)
{
    ITRACE("");

    sockaddr_in serverAddr;

    // Overlapped I/O follows the model established in Windows and can be performed only on 
    // sockets created through the WSASocket function.
    m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

    if (m_Socket == INVALID_SOCKET)
    {
        throw WSAException("Error while opening socket", WSAGetLastError());
    }

    // Cleanup and Init with 0 the serverAddr
    ZeroMemory(&serverAddr, sizeof(serverAddr));

    // Fill up the address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    serverAddr.sin_port = htons(port);

    // Assign local address and port number
    const int bindResult = bind(m_Socket, (struct sockaddr*) & serverAddr, sizeof(serverAddr));
    if (bindResult != 0)
    {
        throw WSAException("Error while binding socket", WSAGetLastError());
    }

    // Make the socket a listening socket
    const int listenResult = listen(m_Socket, SOMAXCONN);
    if (listenResult != 0)
    {
        throw WSAException("Error occurred while listening", WSAGetLastError());
    }

    *SetPort = port;
}


WSAEventWrapper::WSAEventWrapper()
{
    ITRACE("");

    m_Event = WSACreateEvent();
    if (m_Event == WSA_INVALID_EVENT) {
        throw WSAException("unable to create WSA event", WSAGetLastError());
    }
}


WSAEventWrapper::~WSAEventWrapper()
{
    ITRACE("");

    if (!WSACloseEvent(m_Event)) {
        ITRACE("WSACloseEvent failed");
    }
}


const WSAEVENT& WSAEventWrapper::Get() const
{
    //ITRACE("");

    return m_Event;
}
