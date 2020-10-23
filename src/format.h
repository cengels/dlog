#include <string>
#include <time.h>

#pragma once

namespace format {
    /**
     * Formats a POSIX time point as a local time string
     * in the format of hours:minutes:seconds.
     */
    std::string as_local_time_string(const time_t& time);
}
