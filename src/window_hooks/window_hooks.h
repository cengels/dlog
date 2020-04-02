#include <functional>
#include <string>

#pragma once

namespace window_hooks {
    void register_foreground_window_listener(const std::function<void(const std::string&, const std::string&)>& callback);
}