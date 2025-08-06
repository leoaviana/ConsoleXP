// dllmain.cpp : Define o ponto de entrada para o aplicativo DLL.
#include "pch.h"  
#include <iostream>   
#include <string>
#include "Hooks.h"
#include "ActionTarget.h"
#include "CVarHandler.h"
#include <thread>
#include "API.h"
#include "Log.h"

DWORD WINAPI MainThread(LPVOID lpThreadParameter)
{
    Hooks::hModule = (HMODULE)lpThreadParameter;

    Log::Write("ConsoleXP Main thread initialized. Starting hooks...");

    // Invalid function pointer workaround, needed to register cvar callbacks and functions...
    *(DWORD*)0x00D415B8 = 1;
    *(DWORD*)0x00D415BC = 0x7FFFFFFF;

	API::Initialize();
    int result = Hooks::Initialize(); 
    Log::Write("Hook initialization returned %d", result);
     
    // Start a new thread to poll for EndScene device availability
    std::thread(Hooks::WaitForEndSceneHook).detach();

    ActionTarget::Initialize();
    CVarHandler::Initialize();

    return TRUE;
}

DWORD WINAPI ExitThread(LPVOID lpThreadParameter)
{
    if (!Hooks::Detached)
    {
        Hooks::Detached = true;
        FreeLibraryAndExitThread(Hooks::hModule, TRUE);
    }
    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        Log::Initialize();

        if (MH_Initialize() != MH_OK)
        {
            return FALSE;
        }

        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);

        break;

    case DLL_PROCESS_DETACH:
        MH_Uninitialize();
        if (!Hooks::Detached)
            CreateThread(nullptr, 0, ExitThread, hModule, 0, nullptr);
        break;
    }

    return TRUE;
}