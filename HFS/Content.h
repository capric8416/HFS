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
#include <mutex>
#include <set>
#include <string>
#include <stdint.h>


class ContentMeta
{
public:
    ContentMeta();
    ContentMeta(FILE* File, std::string Name, std::string MineType, uint64_t Created);

    bool IsExpired(uint64_t Now);
    std::string& Name();
    long Size();
    std::string Type();

    void ReadFirstBuffer();
    char* GetFirstBuffer(long& Length, int& RangeFrom, int& RangeTo);
    char* ReadOtherBuffer(size_t RangeFrom, size_t& Length);
    void RemoveBuffer(char* Buffer);
    void CloseFile();

private:
    FILE* m_File;
    long m_Size;
    std::string m_Name;
    std::string m_MineType;
    uint64_t m_Created;
    char* m_Buffer;
    std::set<char*> m_Buffers;

    mutable std::mutex m_Mutex;
};


class ContentToServe {
public:
    ContentToServe();
    virtual ~ContentToServe();

    DataRegion Route(EMethod Method, const std::string& URL, const std::string& Range, const std::string& Host, std::string& UUID) const;

    DataRegion GetResouce(const std::string& UUID, int& RangeFrom, int& RangeTo) const;

    ContentMeta* GetMeta(std::string UUID);

    std::string GetOrNewToken(const std::string& Type, const std::string& Path);

    std::string GenUUID();

    void GetRange(const std::string& Range, int& RangeFrom, int& RangeTo) const;

private:
    std::map<std::string, ContentMeta*> m_PathUUID;
    std::map<std::string, ContentMeta*> m_UUIDPath;
};
