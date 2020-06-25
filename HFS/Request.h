#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// project
#include "DataRegion.h"

// c/c++
#include <cassert>
#include <cstring>
#include <map>
#include <string>


enum class EStatus {
    Incomplete,
    OK,
    Fail
};


enum class EMethod {
    Unknown,
    Get,
    Head,
    Post,
};



class MemoryInput {
public:
    MemoryInput();
    explicit MemoryInput(const DataRegion& Region);
    explicit MemoryInput(const char* Data);

    bool HasMoreData() const;
    char ReadChar();

private:
    const char* m_Pos;
    const char* m_End;
};


class HttpRequest {
public:
    HttpRequest();

    EStatus GetStatus() const;

    bool IsComplete() const;

    EMethod GetMethod() const;
    const std::string& GetURI() const;
    std::string GetHeader(std::string Name);

    void Read(MemoryInput* In);

private:
    void Consume(char Ch);

    void ProcessLine();

    void ParseFirstLine();
    void ParseOtherLines();

    // parsing
    EStatus m_Status;
    size_t m_LineNumber;
    std::string m_Line;
    bool m_LastCharWasCR;

    // already parsed
    EMethod m_Method;
    std::string m_URI;
    std::map<std::string, std::string> m_Headers;
};
