// self
#include "Client.h"

// windows
#include <WinSock2.h>


void HttpClient::DoProcessRequest()
{
    ITRACE("");

    assert(!m_Request.IsComplete());

    m_Request.Read(&m_MemoryInput);
    if (!m_Request.IsComplete())
    {
        DoRecv();
    }
    else
    {
        MakeResponse();
        DoSend();
    }
}


void HttpClient::DoRecv()
{
    ITRACE("");

    m_RecvFlags = 0;
    ZeroMemory(&m_OverlappedStruct, sizeof(m_OverlappedStruct));

    m_CurrentOp = EOperation::RecvPending;
    const int result = WSARecv(m_Socket.Get(), &m_RecvBuf, 1, NULL, &m_RecvFlags, &m_OverlappedStruct, NULL);
    const int error = WSAGetLastError();

    if (result == 0 || (result == SOCKET_ERROR && error == WSA_IO_PENDING))
    {
        ITRACE("Pending Recv: %d, error: %d", result, error);
    }
    else
    {
        m_CurrentOp = EOperation::No;
        throw WSAException("WSARecv failed", error);
    }
}


void HttpClient::DoRecvDone(DWORD BytesReceived)
{
    ITRACE("");

    // BytesReceived and RecvData is assumed to be valid
    m_MemoryInput = MemoryInput(DataRegion(m_RecvData.data(), BytesReceived));
    DoProcessRequest();
}


void HttpClient::MakeResponse()
{
    ITRACE("");

    assert(m_Request.IsComplete());

    if (m_Request.GetStatus() == EStatus::OK)
    {
        std::string uuid;
        DataRegion region = m_Content.Route(m_Request.GetMethod(), m_Request.GetURI(), m_Request.GetHeader("Range"), m_Request.GetHeader("Host"), uuid);
        if (region)
        {
            m_Response.Init(region.GetType(), region.GetData(), region.GetLength(), region.GetTotalLength(), region.GetRangeFrom(), region.GetRangeTo(), ((ContentToServe&)m_Content).GetMeta(uuid));
        }
        else
        {
            m_Response.Init(ECode::NotFound);
        }
    }
    else
    {
        m_Response.Init(ECode::BadRequest);
    }
}


void HttpClient::DoSend()
{
    ITRACE("");

    m_SendFlags = 0;
    ZeroMemory(&m_OverlappedStruct, sizeof(m_OverlappedStruct));

    char* buffer = m_Response.Prepare(OTHER_SEND_BUF_LEN);
    if (buffer != nullptr)
    {
        m_Buffers.insert(buffer);
    }

    assert(!m_Response.IsFullySent());
    WSABUF* bufs = m_Response.GetBuffers();
    DWORD bufCount = m_Response.GetBufferCount();

    m_CurrentOp = EOperation::SendPending;
    const int result = WSASend(m_Socket.Get(), bufs, bufCount, NULL, m_SendFlags, &m_OverlappedStruct, NULL);
    const int error = WSAGetLastError();

    if (result == 0 || (result == SOCKET_ERROR && error == WSA_IO_PENDING))
    {
        ITRACE("Pending Send");
    }
    else
    {
        m_CurrentOp = EOperation::No;
        throw WSAException("WSASend failed", error);
    }
}


void HttpClient::DoSendDone(DWORD BytesSent)
{
    m_Response.Advance(BytesSent);

    m_Response.RemoveBuffer();

    bool finished = m_Response.IsFullySent();

    ITRACE("send: %d bytes, finish: %d", BytesSent, finished);

    if (finished)
    {
        // new request
        m_Request = HttpRequest();
        DoProcessRequest();
    }
    else
    {
        DoSend();
    }
}
