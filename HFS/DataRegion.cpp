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


DataRegion::DataRegion(const char* Data, size_t Length)
    : m_Data(Data)
    , m_Length(Length)
    , m_RangeFrom(0)
    , m_RangeTo(0)
    , m_TotalLength(0)
{
}



DataRegion::DataRegion(std::string Type, const char* Data, size_t Length, size_t RangeFrom, size_t RangeTo, long TotalLength)
    : m_Type(Type)
    , m_Data(Data)
    , m_Length(Length)
    , m_RangeFrom(RangeFrom)
    , m_RangeTo(RangeTo)
    , m_TotalLength(TotalLength)
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


long DataRegion::GetTotalLength() const
{
    return m_TotalLength;
}
