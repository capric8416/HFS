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

    std::string FileAccessAuth(std::string Type, std::string Path);

    void InitNewClient(SocketWrapper&& Socket);

    void NotifyClientDeath(size_t ClientID);

    const ClientPtr& GetClientByID(size_t ClientID);

    void ShutDown();

    bool NeedToShutDown() const;

    const ContentToServe& GetContentToServe() const;

    HANDLE GetIOCompletionPort() const;

    size_t GetActiveClientCount() const;

    void EnumClients() const;

private:
    void AddToMap(size_t ClientID, ClientPtr&& Client);

    void RemoveFromMap(size_t ClientID);

private:
    std::atomic<bool> m_ShutdownFlag;
    ContentToServe m_ContentToServe;

    size_t m_ClientCounter;

    mutable std::mutex m_Mutex;
    std::map<size_t, ClientPtr> m_ActiveClients;

    ClientPtr m_NullPtr;

    HANDLE m_IOCompletionPort;
};

