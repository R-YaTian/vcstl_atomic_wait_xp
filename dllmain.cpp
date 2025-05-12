#include "pch.h"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinst);
        break;

    case DLL_PROCESS_DETACH:
        if (reserved) break;
        break;
    }
    return TRUE;
}
