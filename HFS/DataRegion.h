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
    DataRegion(const char* data, size_t length);
    DataRegion(std::string type, const char* data, size_t length, size_t range_from, size_t range_to, long total_length);

    explicit operator bool() const;

    std::string GetType() const;
    const char* GetData() const;
    size_t GetLength() const;
    size_t GetRangeFrom() const;
    size_t GetRangeTo() const;
    long GetTotalLenght() const;

private:
    std::string m_Type;
    const char* m_Data;
    size_t m_Length;
    size_t m_RangeFrom;
    size_t m_RangeTo;
    long m_TotalLength;
};
