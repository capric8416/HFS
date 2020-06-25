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
    HttpServer(uint16_t Port);

    void Run(uint16_t* Port);
    void Stop();

    uint16_t Port();
    std::string FileAccessAuth(std::string Type, std::string Path);

    void AcceptingLoop(uint16_t* Port);

    void AcceptConnection(const ListeningSocketWrapper& ListeningSocket);

private:
    uint16_t m_Port;
    WSAContext m_WSAContext;
    ServerContext m_ServerContext;
};

