#include <windows.h>
#include <iostream>
#include <string>
#include <functional>
#include <psapi.h>
#include "window_hooks.h"
#include "../tracker.h"

HANDLE termination_event;
MSG last_message;
DWORD D_MAX_PATH = MAX_PATH;

void CALLBACK window_event_callback(HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
     if (dwEvent == EVENT_SYSTEM_FOREGROUND) {
         TCHAR process_name[MAX_PATH] = { 0 };
         DWORD process_id = 0;

         GetWindowThreadProcessId(hwnd, &process_id);

         HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);

         if (QueryFullProcessImageName(process_handle, 0, process_name, &D_MAX_PATH)) {
             tracker::handle_new_foreground_process(process_name);
         } else {
             std::cout << "Could not retrieve process title. Error code: " << GetLastError() << std::endl;
         }

         CloseHandle(process_handle);
     }
}

BOOL WINAPI console_ctrl_handler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT ||
        dwCtrlType == CTRL_BREAK_EVENT ||
        dwCtrlType == CTRL_CLOSE_EVENT)
    {
        std::cout << "Terminating tracker..." << std::endl;
        SetEvent(termination_event);

        PostQuitMessage(0);
    }

    return FALSE;
}

void window_hooks::register_foreground_window_listener(const std::function<void(const std::string&, const std::string&)>& callback)
{
    std::cout << "Now tracking..." << std::endl;
    SetWinEventHook(EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND, NULL,
        &window_event_callback,
        0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    // The application should stop anyway, but just in case it doesn't,
    // here's a handler that forces it to close.
    termination_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);

    while (GetMessage(&last_message, NULL, 0, 0) > 0) {
        TranslateMessage(&last_message);
        DispatchMessage(&last_message);
    }
}
