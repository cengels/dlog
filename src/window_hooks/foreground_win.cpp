#include <windows.h>
#include <iostream>
#include <string>
#include <functional>
#include "window_hooks.h"

void CALLBACK WinEventProcCallback(HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
     if (dwEvent == EVENT_SYSTEM_FOREGROUND) {
         std::cout << "test succeeded" << std::endl;
     }
}

void window_hooks::register_foreground_window_listener(const std::function<void(const std::string&, const std::string&)>& callback) {
    SetWinEventHook(EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND, NULL,
        // [&callback](HWINEVENTHOOK hook, DWORD event, HWND hwnd,
        //     LONG idObject, LONG idChild,
        //     DWORD dwEventThread, DWORD dwmsEventTime) -> VOID CALLBACK {
        //         callback("ok", "yes");
        // },
        &WinEventProcCallback,
        0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
}
