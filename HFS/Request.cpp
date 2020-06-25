// self
#include "Request.h"

// project
#include "Common.h"

// windows
#include <Windows.h>


MemoryInput::MemoryInput()
    : m_Pos(0)
    , m_End(0)
{
}


MemoryInput::MemoryInput(const DataRegion& Region)
    : m_Pos(Region.GetData())
    , m_End(Region.GetData() + Region.GetLength())
{
}


MemoryInput::MemoryInput(const char* Data)
    : MemoryInput(DataRegion(Data, strlen(Data)))
{
}



bool MemoryInput::HasMoreData() const
{
    return m_Pos != m_End;
}


char MemoryInput::ReadChar()
{
    assert(HasMoreData());
    const char ch = *m_Pos;
    ++m_Pos;
    return ch;
}


HttpRequest::HttpRequest()
    : m_Status(EStatus::Incomplete)
    , m_LineNumber(0)
    , m_LastCharWasCR(false)
    , m_Method(EMethod::Unknown)
{
}


EStatus HttpRequest::GetStatus() const
{
    return m_Status;
}


bool HttpRequest::IsComplete() const
{
    return m_Status != EStatus::Incomplete;
}


EMethod HttpRequest::GetMethod() const
{
    return m_Method;
}


const std::string& HttpRequest::GetURI() const
{
    return m_URI;
}


std::string HttpRequest::GetHeader(std::string Name)
{
    auto iter = m_Headers.find(Name);
    return iter != m_Headers.end() ? iter->second : "";
}


void HttpRequest::Read(MemoryInput* In)
{
    while (m_Status == EStatus::Incomplete && In->HasMoreData())
    {
        Consume(In->ReadChar());
    }
}


void HttpRequest::Consume(char Ch)
{
    assert(m_Status == EStatus::Incomplete);

    if (!m_LastCharWasCR) {
        if (Ch == '\r') {
            m_LastCharWasCR = true;
        }
        else {
            m_Line += Ch;
        }

    }
    else {
        if (Ch == '\n') {
            m_LastCharWasCR = false;
            ProcessLine();
            m_Line.clear();

        }
        else if (Ch == '\r') {
            m_LastCharWasCR = true;
            m_Line += '\r'; // for prev. CR

        }
        else {
            m_LastCharWasCR = false;
            m_Line += '\r'; // for prev. CR
            m_Line += Ch;
        }
    }
}


void HttpRequest::ProcessLine()
{
    ITRACE("%s", m_Line.c_str());

    if (m_Line.empty()) 
    {
        if (m_LineNumber > 0) 
        {
            m_Status = EStatus::OK;
        }
        else 
        {
            m_Status = EStatus::Fail;
        }
    }
    else {
        if (m_LineNumber == 0) 
        {
            ParseFirstLine();
        }
        else 
        {
            ParseOtherLines();
        }
    }
    ++m_LineNumber;
}


void HttpRequest::ParseFirstLine()
{
    size_t pos = m_Line.find(' ');
    if (pos == std::string::npos) {
        m_Status = EStatus::Fail;
        return;
    }

    const std::string method = m_Line.substr(0, pos);
    if (method == "GET") {
        m_Method = EMethod::Get;
    }
    else if (method == "HEAD") {
        m_Method = EMethod::Head;
    }
    else if (method == "POST") {
        m_Method = EMethod::Post;
    }
    else {
        m_Status = EStatus::Fail;
        return;
    }

    m_Headers["methhod"] = method;

    m_URI = m_Line.substr(pos + 1);
    pos = m_URI.find(' ');
    if (pos != std::string::npos) {
        m_Headers["http_version"] = m_URI.substr(pos + 6);

        // cut http version like HTTP/1.1
        m_URI.resize(pos);
    }

    m_Headers["uri"] = m_URI;
}


void HttpRequest::ParseOtherLines()
{
    size_t pos = m_Line.find(':');
    const std::string key = m_Line.substr(0, pos);
    const std::string value = m_Line.substr(pos + 2);
    m_Headers[key] = value;
}

