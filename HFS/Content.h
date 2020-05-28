#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// project
#include "DataRegion.h"
#include "Request.h"

// c/c++
#include <memory>
#include <map>
#include <string>
#include <stdint.h>


class ContentMeta
{
public:
    ContentMeta();
    ContentMeta(FILE* fp, std::string name, std::string mine_type, long size, uint64_t created, char* buffer);

    bool IsExpired(uint64_t now);
    std::string& Name();
    long Size();
    std::string Type();

    char* ReadBuffer(long& length, int& range_from, int& range_to);
    void CloseFile();

private:
    FILE* m_File;
    long m_Size;
    std::string m_Name;
    std::string m_MineType;
    uint64_t m_Created;
    char* m_Buffer;
};


class ContentToServe {
public:
    ContentToServe();
    ContentToServe(std::size_t bytes);
    ~ContentToServe();

    DataRegion Route(EMethod method, const std::string& url, const std::string& range, const std::string& host) const;

    DataRegion GetResouce(const std::string& uuid, int& range_from, int& range_to) const;

    std::string GetOrNewToken(const std::string& type, const std::string& path);

    std::string GenUUID();

    void GetRange(const std::string& range, int& range_from, int& range_to) const;

private:
    std::size_t m_Bytes;
    std::map<std::string, ContentMeta> m_PathUUID;
    std::map<std::string, ContentMeta> m_UUIDPath;
};
