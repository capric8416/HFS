// self
#include "Server.h"

// project
#include "Common.h"
#include "Exception.h"
#include "Worker.h"

// windows
#include <Windows.h>

// c/c++
#include <thread>



HttpServer::HttpServer()
    : m_Port(34567)
{
}


HttpServer::HttpServer(uint16_t port)
    : m_Port(port)
{
}


void HttpServer::Run(uint16_t* port)
{
    ITRACE("");

    std::thread th([this] { WorkerThreadWrapper(&m_ServerContext); });

    while (true)
    {
        try
        {
            AcceptingLoop(port);
        }
        catch (const WSAException & ex)
        {
            ITRACE("WSA error code: %0x, what: %s", ex.GetErrorCode(), ex.what());
            m_Port++;
            continue;
        }

        break;
    }

    th.join();
}


void HttpServer::Stop()
{
    m_ServerContext.ShutDown();
}


uint16_t HttpServer::Port()
{
    return m_Port;
}


std::string HttpServer::FileAccessAuth(std::string type, std::string path)
{
    return m_ServerContext.FileAccessAuth(type, path);
}


void HttpServer::AcceptingLoop(uint16_t* port)
{
    ITRACE("");

    ListeningSocketWrapper listeningSocket(m_Port, port);
    WSAEventWrapper acceptEvent;
    const DWORD acceptTimeout = 100;

    if (WSAEventSelect(listeningSocket.Get(), acceptEvent.Get(), FD_ACCEPT) == SOCKET_ERROR) {
        throw WSAException("WSAEventSelect failed", WSAGetLastError());
    }

    WSANETWORKEVENTS wsaEvents;
    //StatPrinter stat;

    while (!m_ServerContext.NeedToShutDown())
    {
        //if (stat.Print(ServerContext.GetActiveClientCount())) {
        //    //ServerContext.EnumClients();
        //}

        const DWORD waitRes = WSAWaitForMultipleEvents(1, &acceptEvent.Get(), FALSE, acceptTimeout, FALSE);
        if (waitRes == WSA_WAIT_EVENT_0) {
            if (WSAEnumNetworkEvents(listeningSocket.Get(), acceptEvent.Get(), &wsaEvents) == SOCKET_ERROR)
            {
                throw WSAException("WSAEnumNetworkEvents failed", WSAGetLastError());
            }

            if ((wsaEvents.lNetworkEvents & FD_ACCEPT) && (0 == wsaEvents.iErrorCode[FD_ACCEPT_BIT]))
            {
                AcceptConnection(listeningSocket);
            }

        }
        else if (waitRes == WSA_WAIT_FAILED) {
            throw WSAException("WSAWaitForMultipleEvents failed", WSAGetLastError());
        }
    }
}


void HttpServer::AcceptConnection(const ListeningSocketWrapper& listeningSocket)
{
    ITRACE("");

    sockaddr_in clientAddr;
    ZeroMemory(&clientAddr, sizeof(clientAddr));
    int clientAddrSize = sizeof(clientAddr);

    SocketWrapper socket = WSAAccept(listeningSocket.Get(), reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize, NULL, NULL);
    if (!socket)
    {
        throw WSAException("WSAAccept failed", WSAGetLastError());
    }
    m_ServerContext.InitNewClient(std::move(socket));
}
