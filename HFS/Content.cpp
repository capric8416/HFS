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



ContentMeta::ContentMeta()
{
}


ContentMeta::ContentMeta(FILE* fp, std::string name, std::string mine_type, long size, uint64_t created) :
    m_File(fp),
    m_Name(name),
    m_MineType(mine_type),
    m_Size(size),
    m_Created(created)
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


long ContentMeta::ReadFile(char* buffer, std::size_t& range_from, std::size_t& range_to)
{
    long length = 0;

    if ((range_from == 0 && range_to == 0) || range_from >= m_Size)
    {
        return m_Size;
    }

    if (range_to > m_Size)
    {
        range_to = m_Size;
    }

    rewind(m_File);
    fseek(m_File, range_from, SEEK_CUR);
    length = fread(buffer, sizeof(char), range_to - range_from, m_File);

    range_to -= 1;

    return length;
}


void ContentMeta::CloseFile()
{
    if (m_File != nullptr)
    {
        fclose(m_File);
        m_File = nullptr;
    }
}



ContentToServe::ContentToServe()
    : m_Bytes(512000)
{
    ITRACE("");
}


ContentToServe::ContentToServe(std::size_t bytes)
    : m_Bytes(bytes)
{
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
    ITRACE("");

    if (url.empty() || url[0] != '/') {
        return DataRegion();
    }

    const std::string path = url[url.length() - 1] != '/' ? url.substr(1) : url.substr(1, url.length() - 2);

    switch (method)
    {
        case EMethod::Get:
        {
            std::size_t range_from = 0;
            std::size_t range_to = 0;
            GetRange(range, range_from, range_to);

            return GetResouce(path, range_from, range_to);
        }
        case EMethod::Post:
        {
            auto hostname = host.substr(0, 9);
            if (path.substr(0, 5) == "token" && (hostname == "127.0.0.1" || hostname == "localhost"))
            {
                auto temp = path.substr(6);
                size_t pos = temp.find("/");
                auto type = temp.substr(0, pos);
                auto file = temp.substr(pos + 1);
                return ((ContentToServe*)this)->GetToken(type, file);
            }
        }
    }

    return DataRegion();
}


DataRegion ContentToServe::GetResouce(const std::string& uuid, std::size_t& range_from, std::size_t& range_to) const
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

    char* buffer = new char[m_Bytes];
    long length = meta.ReadFile(buffer, range_from, range_to);
    return DataRegion(meta.Type(), buffer, length, range_from, range_to, meta.Size());
}


DataRegion ContentToServe::GetToken(const std::string& type, const std::string& path)
{
    auto uuid = GetOrNewToken(type, path);
    if (uuid.empty())
    {
        return DataRegion();
    }
    else
    {
        size_t lenght = uuid.length();
        char* buffer = new char[lenght];
        memcpy(buffer, uuid.c_str(), lenght);
        return DataRegion(buffer, lenght);
    }
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
    ContentMeta pathMeta(fp, path, mine_type, size, now);
    ContentMeta uuidMeta(fp, uuid, mine_type, size, now);
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


void ContentToServe::GetRange(const std::string& range, std::size_t& range_from, std::size_t& range_to) const
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
                range_to = str_range_to.empty() ? range_from + m_Bytes : std::atoi(str_range_to.c_str());
            }
        }
    }
}
