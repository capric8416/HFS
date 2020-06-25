// self
#include "Context.h"

// project
#include "Common.h"
#include "Exception.h"

// windows
#include <Windows.h>


WSAContext::WSAContext()
{
    ITRACE("");

    WSADATA wsaData;
    const int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        throw WSAException("WSAStartup failed", WSAGetLastError());
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        WSACleanup();
        throw std::runtime_error("Could not find a usable version of Winsock.dll");
    }
    //std::cerr << "WSA 2.2 Initialized" << std::endl;
}


WSAContext::~WSAContext()
{
    ITRACE("");

    if (WSACleanup() != 0)
    {
        ITRACE("WSACleanup failed");
    }
}


ServerContext::ServerContext()
    : m_ClientCounter(0)
    , m_ShutdownFlag(false)
{
    ITRACE("");

    m_IOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
    if (m_IOCompletionPort == INVALID_HANDLE_VALUE) {
        throw WinAPIException("CreateIoCompletionPort (IOCP creation failed)", GetLastError());
    }
}


ServerContext::~ServerContext()
{
    ITRACE("");

    CloseHandle(m_IOCompletionPort);
}


std::string ServerContext::FileAccessAuth(std::string Type, std::string Path)
{
    return m_ContentToServe.GetOrNewToken(Type, Path);
}


void ServerContext::InitNewClient(SocketWrapper&& Socket)
{
    ITRACE("");

    // Associate the socket with IOCP
    ++m_ClientCounter;
    const size_t clientID = m_ClientCounter;

    const HANDLE hTemp = CreateIoCompletionPort((HANDLE)Socket.Get(), m_IOCompletionPort, clientID, 0);
    if (hTemp == NULL)
    {
        throw WinAPIException("CreateIoCompletionPort (association failed)", GetLastError());
    }
    assert(hTemp == m_IOCompletionPort);

    AddToMap(clientID, ClientPtr(new HttpClient(m_ContentToServe, std::move(Socket))));
    const auto& client = GetClientByID(clientID);

    try
    {
        client->InitialReceive();
    }
    catch (const WinAPIException & ex)
    {
        // assume that we have no pending IO
        ITRACE("ERROR: initial Recv on client: %0x, failed: %s", clientID, ex.what());
        if (client->IsCompleted()) {
            RemoveFromMap(clientID);
        }
    }
}


void ServerContext::NotifyClientDeath(size_t ClientID)
{
    ITRACE("");

    RemoveFromMap(ClientID);
}


const ClientPtr& ServerContext::GetClientByID(size_t ClientID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    const auto it = m_ActiveClients.find(ClientID);
    if (it != m_ActiveClients.end())
    {
        return it->second;
    }
    else
    {
        return m_NullPtr;
    }
}


void ServerContext::ShutDown()
{
    ITRACE("");

    m_ShutdownFlag = true;
}


bool ServerContext::NeedToShutDown() const
{
    return m_ShutdownFlag;
}


const ContentToServe& ServerContext::GetContentToServe() const
{
    ITRACE("");

    return m_ContentToServe;
}


HANDLE ServerContext::GetIOCompletionPort() const
{
    return m_IOCompletionPort;
}


size_t ServerContext::GetActiveClientCount() const
{
    ITRACE("");

    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_ActiveClients.size();
}


void ServerContext::EnumClients() const
{
    ITRACE("");

    std::lock_guard<std::mutex> lock(m_Mutex);
    for (const auto& it : m_ActiveClients) {
        ITRACE("* %d - %d : %d st %d", it.first, it.second->IsCompleted(), it.second->GetTotalBytesTrasferred(), (int)it.second->GetOperation());
    }
}


void ServerContext::AddToMap(size_t ClientID, ClientPtr&& Client)
{
    ITRACE("");

    std::lock_guard<std::mutex> lock(m_Mutex);
    m_ActiveClients[ClientID] = std::move(Client);
}


void ServerContext::RemoveFromMap(size_t ClientID)
{
    ITRACE("");

    std::lock_guard<std::mutex> lock(m_Mutex);
    const auto it = m_ActiveClients.find(ClientID);
    if (it != m_ActiveClients.end())
    {
        m_ActiveClients.erase(it);
    }
}
