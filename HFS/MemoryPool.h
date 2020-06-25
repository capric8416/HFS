#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// c/++
#include <memory>
#include <functional>
#include <vector>
#include <cassert>
#include <stdexcept>


class MemoryPool {
public:
    MemoryPool(size_t BlockSize, size_t Capacity);
    virtual ~MemoryPool();

    size_t GetAvailableBlocks() const;

    char* NewRaw();
    void DeleteRaw(char* Block);

    typedef std::unique_ptr<char, std::function<void(char*)>> Holder;
    Holder Create();

private:
    MemoryPool(const MemoryPool& Other) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

private:
    char* m_Data;
    size_t m_BlockSize;
    size_t m_TotalFree;
    size_t m_FirstFree;
    size_t m_Undefined;
    std::vector<size_t> m_NextFree;
};
