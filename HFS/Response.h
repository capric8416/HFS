#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// windows
#include <Winsock2.h>

// c/c++
#include <array>
#include <sstream>
#include <string>
#include <utility>


class ContentMeta;


enum class ECode {
    OK,
    NotFound,
    BadRequest,
};


class HttpResponse {
public:
    HttpResponse();

    void Init(ECode Code);
    void Init(std::string Type, const char* Data, size_t Length, long RangeMax = 0, size_t RangeFrom = 0, size_t RangeTo = 0, ContentMeta* Meta = nullptr);

    size_t GetTotalSize() const;

    void Advance(size_t Sent);
    void RemoveBuffer();
    void RemoveBuffer(char* Buffer);

    bool IsFullySent() const;

    char* Prepare(size_t MaxBytesToSend);

    WSABUF* GetBuffers() const;
    DWORD GetBufferCount() const;

private:
    void DoInit(ECode Code, std::string Type = "", const char* Data = nullptr, size_t Length = 0, long RangeMax = 0, size_t RangeFrom = 0, size_t RangeTo = 0);

    void WriteCustomBody(int Code, const char* Message);

    void SetBody(const char* Data, size_t Length);


private:
    ECode m_Code;

    std::string m_HeaderText;
    std::string m_CustomBodyText;

    enum {
        Header = 0,
        Body = 1,
    };
    std::array<WSABUF, 2> m_AllBuffers;

    size_t m_Pos;

    ContentMeta* m_Meta;
    long m_RangeFrom;

    DWORD m_SendBufferCount;
    std::array<WSABUF, 2> m_SendBuffers;
};
