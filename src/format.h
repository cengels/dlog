#include <string>
#include <time.h>

#pragma once

namespace format {
    namespace colorize {
        std::ostream& error(std::ostream& stream, const std::string& string);
        std::ostream& activity(std::ostream& stream, const std::string& string);
        std::ostream& project(std::ostream& stream, const std::string& string);
        std::ostream& time(std::ostream& stream, const std::string& string);
        std::ostream& tag(std::ostream& stream, const std::string& string);
        std::ostream& command(std::ostream& stream, const std::string& string);
    }

    /**
     * Formats a POSIX time point as a local time string
     * in the format of hours:minutes:seconds.
     */
    std::string as_local_time_string(const time_t& time);
}
