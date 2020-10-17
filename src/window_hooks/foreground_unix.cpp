#include <iostream>
#include "window_hooks.h"

void window_hooks::register_foreground_window_listener(const std::function<void(const std::string&, const std::string&)>& callback)
{
    std::cout << "Attaching window listeners is not currently supported under Unix.";
}
