#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// project
#include "Client.h"

// c/c++
#include <atomic>
#include <cassert>
#include <map>
#include <mutex>


class WSAContext
{
public:
    WSAContext();
    virtual ~WSAContext();
};


class ServerContext 
{
public:
    ServerContext();
    virtual ~ServerContext();

    std::string FileAccessAuth(std::string type, std::string path);

    void InitNewClient(SocketWrapper&& socket);

    void NotifyClientDeath(size_t clientID);

    const ClientPtr& GetClientByID(size_t clientID);

    void ShutDown();

    bool NeedToShutDown() const;

    const ContentToServe& GetContentToServe() const;

    HANDLE GetIOCompletionPort() const;

    size_t GetActiveClientCount() const;

    void EnumClients() const;

private:
    void AddToMap(size_t clientID, ClientPtr&& client);

    void RemoveFromMap(size_t clientID);

private:
    std::atomic<bool> m_ShutdownFlag;
    ContentToServe m_ContentToServe;

    size_t m_ClientCounter;

    mutable std::mutex m_Mutex;
    std::map<size_t, ClientPtr> m_ActiveClients;

    ClientPtr m_NullPtr;

    HANDLE m_IOCompletionPort;
};

