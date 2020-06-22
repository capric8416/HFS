// self
#include "Content.h"

// project
#include "Common.h"

// windows
#include <Windows.h>
#include <rpc.h>

// c/c++
#include <algorithm>
#include <vector>
#include <sstream>


// Need to link with rpcrt4.lib for uuid
#pragma comment(lib, "rpcrt4.lib")


#define EMPTY_BLOCK 4096


ContentMeta::ContentMeta()
{
}


ContentMeta::ContentMeta(FILE* fp, std::string name, std::string mine_type, long size, uint64_t created, char* buffer)
    : m_File(fp)
    , m_Name(name)
    , m_MineType(mine_type)
    , m_Size(size)
    , m_Created(created)
    , m_Buffer(buffer)
{
}


bool ContentMeta::IsExpired(uint64_t now)
{
    return now - m_Created > 86400000;
}


std::string& ContentMeta::Name()
{
    return m_Name;
}


long ContentMeta::Size()
{
    return m_Size;
}


std::string ContentMeta::Type()
{
    return m_MineType;
}


char* ContentMeta::ReadBuffer(long& length, int& range_from, int& range_to)
{
    length = 0;

    if (range_from >= m_Size)
    {
        return m_Buffer;
    }

    if (range_to > m_Size)
    {
        range_to = m_Size;
    }

    int size = range_to - range_from;
    if (size > 0)
    {
        length = size;
        range_to -= 1;
    }
    else
    {
        length = EMPTY_BLOCK;
    }

    return m_Buffer + range_from * sizeof(char);
}


void ContentMeta::CloseFile()
{
    if (m_File != nullptr)
    {
        fclose(m_File);
        m_File = nullptr;
    }

    if (m_Buffer != nullptr)
    {
        delete[] m_Buffer;
        m_Buffer = nullptr;
    }
}



ContentToServe::ContentToServe()
{
    ITRACE("");
}


ContentToServe::~ContentToServe()
{
    ITRACE("");

    for (auto iter = m_PathUUID.begin(); iter != m_PathUUID.end(); ++iter)
    {
        iter->second.CloseFile();
    }

    m_PathUUID.clear();
    m_UUIDPath.clear();
}


DataRegion ContentToServe::Route(EMethod method, const std::string& url, const std::string& range, const std::string& host) const
{
    if (url.empty() || url[0] != '/') {
        return DataRegion();
    }

    const std::string path = url[url.length() - 1] != '/' ? url.substr(1) : url.substr(1, url.length() - 2);

    switch (method)
    {
        case EMethod::Get:
        {
            int range_from = 0;
            int range_to = 0;
            GetRange(range, range_from, range_to);

            return GetResouce(path, range_from, range_to);
        }
        case EMethod::Post:
        {
        }
    }

    return DataRegion();
}


DataRegion ContentToServe::GetResouce(const std::string& uuid, int& range_from, int& range_to) const
{
    auto iter = m_UUIDPath.find(uuid);
    if (iter == m_UUIDPath.end())
    {
        return DataRegion();
    }

    uint64_t now = GetTickCount64();

    auto meta = iter->second;
    if (meta.IsExpired(now))
    {
        return DataRegion();
    }

    if (range_to == -1)
    {
        range_to = meta.Size();
    }

    long length = 0;
    char* buffer = meta.ReadBuffer(length, range_from, range_to);
    return DataRegion(meta.Type(), buffer, length, range_from, range_to, meta.Size());
}


std::string ContentToServe::GetOrNewToken(const std::string& type, const std::string& path)
{
    uint64_t now = GetTickCount64();

    auto iter = m_PathUUID.find(path);
    if (iter != m_PathUUID.end())
    {
        auto meta = iter->second;
        if (!meta.IsExpired(now))
        {
            return meta.Name();
        }
        else
        {
            meta.CloseFile();
            m_PathUUID.erase(iter);

            auto pathIter = m_UUIDPath.find(meta.Name());
            if (pathIter != m_UUIDPath.end())
            {
                m_UUIDPath.erase(pathIter);
            }
        }
    }

    std::string uuid = GenUUID();
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == NULL)
    {
        return "";
    }

    std::string mine_type = type;
    if (type.find("/") == std::string::npos)
    {
        mine_type = type + "/" + path.substr(path.rfind(".") + 1);
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    char* buffer = new char[size];
    rewind(fp);
    fread(buffer, sizeof(char), size, fp);
    ContentMeta pathMeta(fp, path, mine_type, size, now, buffer);
    ContentMeta uuidMeta(fp, uuid, "", 0, now, buffer);
    m_UUIDPath[uuid] = pathMeta;
    m_PathUUID[path] = uuidMeta;

    return uuid;
}


std::string ContentToServe::GenUUID()
{
    // create uuid
    UUID uuid;
    UuidCreate(&uuid);
    char* temp;
    UuidToStringA(&uuid, (RPC_CSTR*)&temp);

    // remove -
    std::string result(temp);
    std::string::iterator end_pos = std::remove(result.begin(), result.end(), '-');
    result.erase(end_pos, result.end());

    // free char*
    RpcStringFreeA((RPC_CSTR*)&temp);

    return result;
}


void ContentToServe::GetRange(const std::string& range, int& range_from, int& range_to) const
{
    if (!range.empty())
    {
        std::string temp = range;
        std::size_t pos = temp.find("=");
        if (pos != temp.npos)
        {
            temp = temp.substr(pos + 1);
            pos = temp.find("-");
            if (pos != temp.npos)
            {
                range_from = std::atoi(temp.substr(0, pos).c_str());
                auto str_range_to = temp.substr(pos + 1);
                range_to = str_range_to.empty() ? -1 : std::atoi(str_range_to.c_str());
            }
        }
    }
}
