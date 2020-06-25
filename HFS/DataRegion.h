#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// c/c++
#include <cstdlib>
#include <string>


class DataRegion {
public:
    DataRegion();
    DataRegion(const char* Data, size_t Length);
    DataRegion(std::string Type, const char* Data, size_t Length, size_t RangeFrom, size_t RangeTo, long TotalLength);

    explicit operator bool() const;

    std::string GetType() const;
    const char* GetData() const;
    size_t GetLength() const;
    size_t GetRangeFrom() const;
    size_t GetRangeTo() const;
    long GetTotalLength() const;

private:
    std::string m_Type;
    const char* m_Data;
    size_t m_Length;
    size_t m_RangeFrom;
    size_t m_RangeTo;
    long m_TotalLength;
};
