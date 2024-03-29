// self
#include "Worker.h"

// project
#include "Common.h"
#include "Exception.h"
#include "Client.h"
#include "Server.h"

// windows
#include <Windows.h>



void WorkerThread(ServerContext* Ctx)
{
    ITRACE("");

    while (!Ctx->NeedToShutDown())
    {
        DWORD bytesTransferred = 0;
        ULONG_PTR clientID = 0;
        OVERLAPPED* overlappedStruct = 0;

        const BOOL result = GetQueuedCompletionStatus(Ctx->GetIOCompletionPort(), &bytesTransferred, &clientID, &overlappedStruct, 2000);

        if (!result || (bytesTransferred == 0))
        {
            if (clientID != 0)
            {
                //Client connection gone, remove it.
                Ctx->NotifyClientDeath(clientID);
                continue;
            }
        }

        const ClientPtr& client = Ctx->GetClientByID(clientID);
        if (client)
        {
            assert(client->IsCompleted());
            try
            {
                client->CompleteIOOperation(bytesTransferred);
            }
            catch (const WinAPIException & ex)
            {
                ITRACE("ERROR: client %0x, failed: %s", clientID, ex.what());
                if (client->IsCompleted())
                {
                    Ctx->NotifyClientDeath(clientID);
                }
            }
        }
    }
}


void WorkerThreadWrapper(ServerContext* Ctx)
{
    ITRACE("");

    try
    {
        WorkerThread(Ctx);
    }
    catch (const std::exception & ex)
    {
        ITRACE("execption:\n%s", ex.what());
        throw;
    }
}
