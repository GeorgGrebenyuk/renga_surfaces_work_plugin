#include <windows.h>

HMODULE DllHandle;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
        DllHandle = hModule;
    return TRUE;
}