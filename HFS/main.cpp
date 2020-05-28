#ifdef _DEBUG


// define
#define WIN32_LEAN_AND_MEAN

// project
#include "Common.h"
#include "Exception.h"
#include "Server.h"

// windows
#include <tchar.h>


// Need to link with Ws2_32.lib for wsascoket
#pragma comment (lib, "Ws2_32.lib")


HttpServer* xfs = nullptr;


int _tmain(int /*argc*/, _TCHAR* /*argv[]*/)
{
    ITRACE("");

    xfs = new HttpServer();

    auto port = xfs->Port();
    auto token = xfs->FileAccessAuth("video", "E:\\Overwatch_Alive_Short_720p_30fps.mp4");
    ITrace("========== http://192.168.11.145:%d/%s", port, token);

    try
    {
        uint16_t port = 0;
        xfs->Run(&port);
    }
    catch (const WSAException & ex)
    {
        ITRACE("WSA error code: %0x, what: %s", ex.GetErrorCode(), ex.what());
    }
    catch (const WinAPIException & ex)
    {
        ITRACE("WinAPI error code: %0x, what: %s", ex.GetErrorCode(), ex.what());
    }
    catch (const std::exception & ex)
    {
        ITRACE("exception: %s", ex.what());
    }

    delete xfs;

    return 0;
}


#endif // _DEBUG