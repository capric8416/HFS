// self
#include "DataRegion.h"

// project
#include "Common.h"

// windows
#include <Windows.h>


DataRegion::DataRegion()
    : m_Data(nullptr)
    , m_Length(0)
    , m_RangeFrom(0)
    , m_RangeTo(0)
    , m_TotalLength(0)
{
}


DataRegion::DataRegion(const char* data, size_t length)
    : m_Data(data)
    , m_Length(length)
    , m_RangeFrom(0)
    , m_RangeTo(0)
    , m_TotalLength(0)
{
}



DataRegion::DataRegion(std::string type, const char* data, size_t length, size_t range_from, size_t range_to, long total_length)
    : m_Type(type)
    , m_Data(data)
    , m_Length(length)
    , m_RangeFrom(range_from)
    , m_RangeTo(range_to)
    , m_TotalLength(total_length)
{
}


DataRegion::operator bool() const 
{
    return m_Data != nullptr || m_TotalLength > 0;
}


std::string DataRegion::GetType() const
{
    return m_Type;
}


const char* DataRegion::GetData() const
{
    return m_Data;
}


size_t DataRegion::GetLength() const 
{
    return m_Length;
}


size_t DataRegion::GetRangeFrom() const
{
    return m_RangeFrom;
}


size_t DataRegion::GetRangeTo() const
{
    return m_RangeTo;
}


long DataRegion::GetTotalLenght() const
{
    return m_TotalLength;
}
