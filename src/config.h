#include <string>
#include <vector>

#pragma once

namespace config {
    struct rule {
        std::vector<std::string> match_process;
        window_state match_window_state;
    };

    struct config {
        std::string output_folder;
        std::string output_file;
        bool filter_by_rules;
        long delay;
        std::vector<rule> rules;
    };

    enum class window_state {
        NONE,
        WINDOWED = 1,
        FULLSCREEN = 2,
        MAXIMIZED = 4,
        MINIMIZED = 8
    };
}
