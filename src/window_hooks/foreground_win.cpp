#include <windows.h>
#include <iostream>
#include <string>
#include <functional>
#include "window_hooks.h"
#include <psapi.h>
HANDLE g_hTerminateEvent;
MSG msg;

void CALLBACK WinEventProcCallback(HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
     if (dwEvent == EVENT_SYSTEM_FOREGROUND) {
         TCHAR buffer[MAX_PATH] = { 0 };
         DWORD dwProcId = 0;
         //HMODULE module;
         DWORD value = MAX_PATH;

         GetWindowThreadProcessId(hwnd, &dwProcId);

         HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcId);

         if (QueryFullProcessImageName(hProc, 0, buffer, &value)) {
             std::cout << "New foreground window is " << buffer << std::endl;
         } else {
             std::cout << "Could not retrieve process title. Error code: " << GetLastError() << std::endl;
         }

         CloseHandle(hProc);
     }
}

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT ||
        dwCtrlType == CTRL_BREAK_EVENT ||
        dwCtrlType == CTRL_CLOSE_EVENT)
    {
        std::cout << "Terminating tracker..." << std::endl;
        SetEvent(g_hTerminateEvent);

        PostQuitMessage(0);
    }

    return FALSE;
}

void window_hooks::register_foreground_window_listener(const std::function<void(const std::string&, const std::string&)>& callback)
{
    std::cout << "Now tracking..." << std::endl;
    SetWinEventHook(EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND, NULL,
        &WinEventProcCallback,
        0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    // The application should stop anyway, but just in case it doesn't,
    // here's a handler that forces it to close.
    g_hTerminateEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
