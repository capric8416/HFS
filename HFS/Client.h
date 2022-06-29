#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// project
#include "Common.h"
#include "Exception.h"
#include "Content.h"
#include "Request.h"
#include "Response.h"
#include "Wrapper.h"

// c/c++
#include <cassert>
#include <stdexcept>

// windows
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN


enum EOperation
{
    No,
    RecvPending,
    SendPending,
};


class HttpClient
{
public:
    HttpClient(const ContentToServe& Content, SocketWrapper&& Socket)
        : m_Content(Content)
        , m_Socket(std::move(Socket))
        , m_CurrentOp(EOperation::No)
        , m_RecvFlags(0)
        , m_TotalBytesTransferred(0)
    {
        ZeroMemory(&m_OverlappedStruct, sizeof(m_OverlappedStruct));

        m_RecvBuf.len = static_cast<unsigned long>(m_RecvData.size());
        m_RecvBuf.buf = m_RecvData.data();
    }

    virtual ~HttpClient()
    {
        for (char* buffer : m_Buffers)
        {
            m_Response.RemoveBuffer(buffer);
        }
        m_Buffers.clear();

        //assert(IsCompleted());
    }

    // is called from accept thread
    void InitialReceive()
    {
        DoRecv();
    }

    // is called from working thread
    void CompleteIOOperation(DWORD BytesTransferred)
    {
        m_TotalBytesTransferred += BytesTransferred;

        switch (m_CurrentOp)
        {
            case EOperation::RecvPending:
                DoRecvDone(BytesTransferred);
                break;
            case EOperation::SendPending:
                DoSendDone(BytesTransferred);
                break;
            default:
                throw std::runtime_error("end of IO operation received, but no op is pending");
        }
    }

    bool IsCompleted() const
    {
        return (m_CurrentOp == EOperation::No) || HasOverlappedIoCompleted(&m_OverlappedStruct);
    }


    size_t GetTotalBytesTrasferred() const
    {
        return m_TotalBytesTransferred;
    }

    EOperation GetOperation() const
    {
        return m_CurrentOp;
    }

private:
    void DoProcessRequest();

    void DoRecv();

    void DoRecvDone(DWORD BytesReceived);

    void MakeResponse();

    void DoSend();

    void DoSendDone(DWORD BytesSent);

private:
    const ContentToServe& m_Content;
    SocketWrapper m_Socket;
    OVERLAPPED m_OverlappedStruct;

    EOperation m_CurrentOp;

    WSABUF m_RecvBuf;
    DWORD m_RecvFlags;
    std::array<char, RECV_BUF_LEN> m_RecvData;
    MemoryInput m_MemoryInput;
    HttpRequest m_Request;

    DWORD m_SendFlags;
    HttpResponse m_Response;

    size_t m_TotalBytesTransferred;

    std::set<char*> m_Buffers;
};


using ClientPtr = std::unique_ptr<HttpClient>;

