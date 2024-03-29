// self
#include "MemoryPool.h"

// project
#include "Common.h"

// windows
#include <Windows.h>


MemoryPool::MemoryPool(size_t BlockSize, size_t Capacity)
    : m_BlockSize(BlockSize)
    , m_TotalFree(Capacity)
    , m_FirstFree(0)
    , m_Undefined(Capacity)
    , m_NextFree(Capacity)
{
    ITRACE("");

    if (Capacity == 0) {
        return;
    }
    m_Data = static_cast<char*>(_aligned_malloc(m_BlockSize * Capacity, m_BlockSize));
    if (m_Data == 0) {
        throw std::bad_alloc();
    }
    for (size_t i = 0; i < m_NextFree.size(); ++i) {
        m_NextFree[i] = i + 1;
    }
    assert(m_NextFree.back() == m_Undefined);
}


MemoryPool::~MemoryPool()
{
    ITRACE("");

    _aligned_free(m_Data);
}


size_t MemoryPool::GetAvailableBlocks() const
{
    ITRACE("");

    return m_TotalFree;
}


char* MemoryPool::NewRaw()
{
    ITRACE("");

    if (m_FirstFree == m_Undefined) 
    {
        throw std::runtime_error("no free blocks available");
    }
    const size_t index = m_FirstFree;
    m_FirstFree = m_NextFree[m_FirstFree];
    --m_TotalFree;
    return m_Data + m_BlockSize * index;
}


void MemoryPool::DeleteRaw(char* Block)
{
    ITRACE("");

    const ptrdiff_t delta = Block - m_Data;
    if (delta < 0 || delta % m_BlockSize != 0) 
    {
        throw std::runtime_error("bad block pointer");
    }

    const size_t index = delta / m_BlockSize;
    if (index >= m_NextFree.size()) 
    {
        throw std::runtime_error("bad block pointer");
    }

    m_NextFree[index] = m_FirstFree;
    m_FirstFree = index;
    ++m_TotalFree;
}


MemoryPool::Holder MemoryPool::Create()
{
    ITRACE("");

    return Holder(NewRaw(), [this](char* block) { DeleteRaw(block); });
}
