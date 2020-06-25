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
    : m_File(nullptr)
    , m_Name("")
    , m_Size(0)
    , m_MineType("")
    , m_Created(0)
    , m_Buffer(nullptr)
{
}


ContentMeta::ContentMeta(FILE* File, std::string Name, std::string MineType, uint64_t Created)
    : m_File(File)
    , m_Name(Name)
    , m_Size(0)
    , m_MineType(MineType)
    , m_Created(Created)
    , m_Buffer(nullptr)
{
}


bool ContentMeta::IsExpired(uint64_t Now)
{
    return Now - m_Created > 86400000;
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


void ContentMeta::ReadFirstBuffer()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    fseek(m_File, 0, SEEK_END);
    m_Size = ftell(m_File);

    m_Buffer = new char[FIRST_SEND_BUF_LEN];
    memset(m_Buffer, 0, FIRST_SEND_BUF_LEN);

    rewind(m_File);
    fread(m_Buffer, sizeof(char), FIRST_SEND_BUF_LEN, m_File);
}


char* ContentMeta::GetFirstBuffer(long& Length, int& RangeFrom, int& RangeTo)
{
    Length = 0;

    if (RangeFrom >= m_Size)
    {
        return m_Buffer;
    }

    if (RangeTo > m_Size)
    {
        RangeTo = m_Size;
    }

    int size = RangeTo - RangeFrom;
    if (size > 0)
    {
        Length = size;
        RangeTo -= 1;
    }
    else
    {
        Length = FIRST_SEND_BUF_LEN;
    }

    return m_Buffer;
}


char* ContentMeta::ReadOtherBuffer(size_t RangeFrom, size_t& Length)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    char* buffer = new char[Length];
    memset(buffer, 0, Length);

    rewind(m_File);
    fseek(m_File, RangeFrom, SEEK_CUR);
    Length = fread(buffer, sizeof(char), Length, m_File);

    m_Buffers.insert(buffer);

    ITRACE("range: %d - %d, buffers: %d", RangeFrom, RangeFrom + Length, m_Buffers.size());

    return buffer;
}


void ContentMeta::RemoveBuffer(char* Buffer)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto iter = m_Buffers.find(Buffer);
    if (iter != m_Buffers.end())
    {
        delete[] Buffer;
        Buffer = nullptr;

        m_Buffers.erase(iter);

        ITRACE("buffers: %d", m_Buffers.size());
    }
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

    for (char* buffer : m_Buffers)
    {
        delete[] buffer;
        buffer = nullptr;
    }
    m_Buffers.clear();
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
        iter->second->CloseFile();
        delete iter->second;
    }
    m_PathUUID.clear();

    for (auto iter = m_UUIDPath.begin(); iter != m_UUIDPath.end(); ++iter)
    {
        delete iter->second;
    }
    m_UUIDPath.clear();
}


DataRegion ContentToServe::Route(EMethod Method, const std::string& URL, const std::string& Range, const std::string& Host, std::string& UUID) const
{
    if (URL.empty() || URL[0] != '/') {
        return DataRegion();
    }

    UUID = URL[URL.length() - 1] != '/' ? URL.substr(1) : URL.substr(1, URL.length() - 2);

    switch (Method)
    {
        case EMethod::Get:
        {
            int rangeFrom = 0;
            int rangeTo = 0;
            GetRange(Range, rangeFrom, rangeTo);

            return GetResouce(UUID, rangeFrom, rangeTo);
        }
        case EMethod::Post:
        {
        }
    }

    return DataRegion();
}


DataRegion ContentToServe::GetResouce(const std::string& UUID, int& RangeFrom, int& RangeTo) const
{
    auto iter = m_UUIDPath.find(UUID);
    if (iter == m_UUIDPath.end())
    {
        return DataRegion();
    }

    uint64_t now = GetTickCount64();

    auto meta = iter->second;
    if (meta->IsExpired(now))
    {
        auto uuidIter = m_PathUUID.find(meta->Name());
        if (uuidIter != m_PathUUID.end())
        {
            uuidIter->second->CloseFile();
            delete uuidIter->second;
            ((std::map<std::string, ContentMeta*>)m_PathUUID).erase(uuidIter);
        }

        delete meta;
        ((std::map<std::string, ContentMeta*>)m_UUIDPath).erase(iter);

        return DataRegion();
    }

    if (RangeTo == -1)
    {
        RangeTo = meta->Size();
    }

    long length = 0;
    char* buffer = meta->GetFirstBuffer(length, RangeFrom, RangeTo);
    return DataRegion(meta->Type(), buffer, length, RangeFrom, RangeTo, meta->Size());
}


ContentMeta* ContentToServe::GetMeta(std::string UUID)
{
    auto iter = m_UUIDPath.find(UUID);
    return iter != m_UUIDPath.end() ? iter->second : nullptr;
}


std::string ContentToServe::GetOrNewToken(const std::string& Type, const std::string& Path)
{
    uint64_t now = GetTickCount64();

    auto iter = m_PathUUID.find(Path);
    if (iter != m_PathUUID.end())
    {
        auto meta = iter->second;
        if (!meta->IsExpired(now))
        {
            return meta->Name();
        }
        else
        {
            auto pathIter = m_UUIDPath.find(meta->Name());
            if (pathIter != m_UUIDPath.end())
            {
                delete pathIter->second;
                m_UUIDPath.erase(pathIter);
            }

            meta->CloseFile();
            delete meta;
            m_PathUUID.erase(iter);
        }
    }

    std::string uuid = GenUUID();
    FILE* fp = fopen(Path.c_str(), "rb");
    if (fp == NULL)
    {
        return "";
    }

    std::string mineType = Type;
    if (Type.find("/") == std::string::npos)
    {
        mineType = Type + "/" + Path.substr(Path.rfind(".") + 1);
    }

    ContentMeta* pathMeta = new ContentMeta(fp, Path, mineType, now);
    ContentMeta* uuidMeta = new ContentMeta(fp, uuid, "", now);

    pathMeta->ReadFirstBuffer();

    m_UUIDPath[uuid] = pathMeta;
    m_PathUUID[Path] = uuidMeta;

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


void ContentToServe::GetRange(const std::string& Range, int& RangeFrom, int& RangeTo) const
{
    if (!Range.empty())
    {
        std::string temp = Range;
        std::size_t pos = temp.find("=");
        if (pos != temp.npos)
        {
            temp = temp.substr(pos + 1);
            pos = temp.find("-");
            if (pos != temp.npos)
            {
                RangeFrom = std::atoi(temp.substr(0, pos).c_str());
                auto str_range_to = temp.substr(pos + 1);
                RangeTo = str_range_to.empty() ? -1 : std::atoi(str_range_to.c_str());
            }
        }
    }
}
