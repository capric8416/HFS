// self
#include "Response.h"

// project
#include "Common.h"

// c/c++
#include <cassert>


HttpResponse::HttpResponse()
    : m_Pos(0)
    , m_SendBufferCount(0)
{
}


void HttpResponse::Init(ECode code)
{
    DoInit(code);
}


void HttpResponse::Init(std::string type, const char* data, size_t length, long range_max, size_t range_from, size_t range_to)
{
    DoInit(ECode::OK, type, data, length, range_max, range_from, range_to);
}


size_t HttpResponse::GetTotalSize() const
{
    size_t sum = 0;
    for (const WSABUF buf : m_AllBuffers)
    {
        sum += buf.len;
    }
    return sum;
}


void HttpResponse::Advance(size_t sent)
{
    m_Pos += sent;
    assert(m_Pos <= GetTotalSize());
}


bool HttpResponse::IsFullySent() const
{
    assert(m_Pos <= GetTotalSize());
    return m_Pos == GetTotalSize();
}


void HttpResponse::Prepare(size_t maxBytesToSend)
{
    m_SendBufferCount = 0;

    size_t acc = 0;
    for (const WSABUF buf : m_AllBuffers) {
        const size_t beg = std::max<size_t>(acc, m_Pos);
        const size_t end = std::min<size_t>(acc + buf.len, m_Pos + maxBytesToSend);
        if (beg < end) {
            m_SendBuffers[m_SendBufferCount].len = static_cast<ULONG>(end - beg);
            m_SendBuffers[m_SendBufferCount].buf = buf.buf + (beg - acc);
            ++m_SendBufferCount;
        }
        acc += buf.len;
    }
}


WSABUF* HttpResponse::GetBuffers() const
{
    return const_cast<WSABUF*>(m_SendBuffers.data());
}


DWORD HttpResponse::GetBufferCount() const
{
    return m_SendBufferCount;
}


void HttpResponse::DoInit(ECode code, std::string type, const char* data, size_t length, long range_max, size_t range_from, size_t range_to)
{
    m_Code = code;

    m_Pos = 0;
    m_SendBufferCount = 0;

    static const char endl[] = "\r\n";

    std::ostringstream oss;
    switch (code) 
    {
        case ECode::OK:
            oss << (range_to == 0 ? "HTTP/1.1 200 OK" : "HTTP/1.1 206 Partial Content") << endl;
            oss << "Connection: Keep-Alive" << endl;
            oss << "Accept-Ranges: bytes" << endl;
            oss << "Content-Type: " + type << endl;
            if (range_to > 0)
            {
                oss << "Content-Range: bytes " << range_from << "-" << range_to << "/" << range_max << endl;
            }
            SetBody(data, length);
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


void HttpResponse::WriteCustomBody(int code, const char* message)
{
    std::ostringstream oss;
    oss << "<html>";
    oss << "<body>";
    oss << "<h1>" << code << " " << message << "</h1>";
    oss << "</body>";
    oss << "</html>";
    m_CustomBodyText = std::move(oss.str());
}


void HttpResponse::SetBody(const char* data, size_t length)
{
    m_AllBuffers[Body].buf = const_cast<char*>(data);
    m_AllBuffers[Body].len = static_cast<ULONG>(length);
}