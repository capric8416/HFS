// self
#include "Response.h"

// project
#include "Common.h"
#include "Content.h"

// c/c++
#include <cassert>


HttpResponse::HttpResponse()
    : m_Pos(0)
    , m_Meta(nullptr)
    , m_RangeFrom(0)
    , m_SendBufferCount(0)
{
}


void HttpResponse::Init(ECode Type)
{
    m_Meta = nullptr;
    m_RangeFrom = 0;
    DoInit(Type);
}


void HttpResponse::Init(std::string Type, const char* Data, size_t Length, long RangeMax, size_t RangeFrom, size_t RangeTo, ContentMeta* Meta)
{
    m_Meta = Meta;
    m_RangeFrom = RangeFrom;
    DoInit(ECode::OK, Type, Data, Length, RangeMax, RangeFrom, RangeTo);
}


size_t HttpResponse::GetTotalSize() const
{
    if (m_Meta != nullptr)
    {
        return m_AllBuffers[Header].len + m_Meta->Size();
    }
    else
    {
        size_t sum = 0;
        for (const WSABUF buf : m_AllBuffers)
        {
            sum += buf.len;
        }
        return sum;
    }
}


void HttpResponse::Advance(size_t Sent)
{
    m_Pos += Sent;
    assert(m_Pos <= GetTotalSize());
}


void HttpResponse::RemoveBuffer()
{
    if (m_Meta != nullptr)
    {
        m_Meta->RemoveBuffer(m_SendBuffers[m_SendBufferCount - 1].buf);
    }
}


void HttpResponse::RemoveBuffer(char* Buffer)
{
    if (m_Meta != nullptr)
    {
        m_Meta->RemoveBuffer(Buffer);
    }
}


bool HttpResponse::IsFullySent() const
{
    assert(m_Pos <= GetTotalSize());
    return m_Pos == GetTotalSize();
}


char* HttpResponse::Prepare(size_t MaxBytesToSend)
{
    char* buffer = nullptr;

    m_SendBufferCount = 0;

    size_t acc = 0;
    for (const WSABUF buf : m_AllBuffers)
    {
        const size_t beg = std::max<size_t>(acc, m_Pos);
        const size_t end = std::min<size_t>(acc + buf.len, m_Pos + MaxBytesToSend);
        if (beg < end)
        {
            size_t length = end - beg;
            if (FIRST_SEND_BUF_LEN < length && length <= OTHER_SEND_BUF_LEN)
            {
                buffer = m_Meta->ReadOtherBuffer(m_RangeFrom + beg - acc, length);

                m_SendBuffers[m_SendBufferCount].buf = buffer;
            }
            else
            {
                m_SendBuffers[m_SendBufferCount].buf = buf.buf + (beg - acc);
            }
            m_SendBuffers[m_SendBufferCount].len = length;

            ++m_SendBufferCount;
        }
        acc += buf.len;
    }

    return buffer;
}


WSABUF* HttpResponse::GetBuffers() const
{
    return const_cast<WSABUF*>(m_SendBuffers.data());
}


DWORD HttpResponse::GetBufferCount() const
{
    return m_SendBufferCount;
}


void HttpResponse::DoInit(ECode Code, std::string Type, const char* Data, size_t Length, long RangeMax, size_t RangeFrom, size_t RangeTo)
{
    m_Code = Code;

    m_Pos = 0;
    m_SendBufferCount = 0;

    static const char endl[] = "\r\n";

    std::ostringstream oss;
    switch (Code)
    {
        case ECode::OK:
            oss << (RangeTo == 0 ? "HTTP/1.1 200 OK" : "HTTP/1.1 206 Partial Content") << endl;
            oss << "Connection: Keep-Alive" << endl;
            oss << "Accept-Ranges: bytes" << endl;
            oss << "Content-Type: " + Type << endl;
            if (RangeTo > 0)
            {
                oss << "Content-Range: bytes " << RangeFrom << "-" << RangeTo << "/" << RangeMax << endl;
            }
            SetBody(Data, Length);
            break;

        case ECode::NotFound:
            oss << "HTTP/1.1 404 Not Found" << endl;
            oss << "Content-Type: text/html" << endl;
            WriteCustomBody(404, "Not Found");
            SetBody(m_CustomBodyText.c_str(), m_CustomBodyText.length());
            break;

        case ECode::BadRequest:
            oss << "HTTP/1.1 400 Bad Request" << endl;
            oss << "Content-Type: text/html" << endl;
            WriteCustomBody(400, "Bad Request");
            SetBody(m_CustomBodyText.c_str(), m_CustomBodyText.length());
            break;
    }
    oss << "Content-Length: " << m_AllBuffers[Body].len << "\r\n";
    oss << "\r\n";

    m_HeaderText = std::move(oss.str());
    ITRACE("\n%s", m_HeaderText.c_str());
    m_AllBuffers[Header].buf = &m_HeaderText[0];
    m_AllBuffers[Header].len = static_cast<ULONG>(m_HeaderText.length());
}


void HttpResponse::WriteCustomBody(int Code, const char* Message)
{
    std::ostringstream oss;
    oss << "<html>";
    oss << "<body>";
    oss << "<h1>" << Code << " " << Message << "</h1>";
    oss << "</body>";
    oss << "</html>";
    m_CustomBodyText = std::move(oss.str());
}


void HttpResponse::SetBody(const char* Data, size_t Length)
{
    m_AllBuffers[Body].buf = const_cast<char*>(Data);
    m_AllBuffers[Body].len = static_cast<ULONG>(Length);
}