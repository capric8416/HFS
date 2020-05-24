#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif // !VC_EXTRALEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

// ITrace like printf
void ITrace(char* Format, ...);



// XTRACE only working on debug mode
#ifndef _DEBUG
#define ITRACE(Format, ...) (void)0
#else
#define BACKSLASH '\\'
#define LAST_BACKSLASH_POS strrchr(__FILE__, BACKSLASH)
#define FILE_NAME LAST_BACKSLASH_POS ? LAST_BACKSLASH_POS + 1 : __FILE__
#define ITRACE(Format, ...)                                     \
        ITrace(                                                 \
            "=====>>>>> [%lld %s %d %s] " ## Format ## "\n",    \
            GetTickCount64(),                                   \
            FILE_NAME,                                          \
            __LINE__,                                           \
            __FUNCTION__,                                       \
            __VA_ARGS__                                         \
        )
#endif // !_DEBUG
