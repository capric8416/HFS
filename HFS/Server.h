#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// project
#include "Wrapper.h"
#include "Context.h"

// c/c++
#include <stdint.h>


class HttpServer
{
public:
    HttpServer();
    HttpServer(uint16_t port);

    void Run(uint16_t* port);
    void Stop();

    uint16_t Port();
    std::string FileAccessAuth(std::string type, std::string path);

    void AcceptingLoop(uint16_t* port);

    void AcceptConnection(const ListeningSocketWrapper& listeningSocket);

private:
    uint16_t m_Port;
    WSAContext m_WSAContext;
    ServerContext m_ServerContext;
};

