#include "rang.hpp"

#pragma once

namespace cli {
    enum class color {
        error = static_cast<int>(rang::fg::red),
        error_message = static_cast<int>(rang::fgB::red),
        project = static_cast<int>(rang::fgB::red),
        activity = static_cast<int>(rang::fg::cyan),
        time = static_cast<int>(rang::fg::magenta),
        duration = static_cast<int>(rang::fgB::magenta),
        date = static_cast<int>(rang::fg::green),
        tag = static_cast<int>(rang::fgB::yellow),
        command = static_cast<int>(rang::fg::blue),
        reset = static_cast<int>(rang::fg::reset)
    };

    std::ostream& operator<<(std::ostream& stream, const cli::color& color);
}
