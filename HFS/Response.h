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


enum class ECode {
    OK,
    NotFound,
    BadRequest,
};


class HttpResponse {
public:
    HttpResponse();

    void Init(ECode code);
    void Init(std::string type, const char* data, size_t length, long range_max = 0, size_t range_from = 0, size_t range_to = 0);

    size_t GetTotalSize() const;

    void Advance(size_t sent);

    bool IsFullySent() const;

    void Prepare(size_t maxBytesToSend);

    WSABUF* GetBuffers() const;
    DWORD GetBufferCount() const;

private:
    void DoInit(ECode code, std::string type = "", const char* data = nullptr, size_t length = 0, long range_max = 0, size_t range_from = 0, size_t range_to = 0);

    void WriteCustomBody(int code, const char* message);

    void SetBody(const char* data, size_t length);


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

    long m_TotalLength;
    DWORD m_SendBufferCount;
    std::array<WSABUF, 2> m_SendBuffers;
};
