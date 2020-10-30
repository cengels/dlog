#include <time.h>
#include <string>

#pragma once

namespace parser {
    /**
     * Attempts to parse the specified date-time, time, or
     * temporal expression. If it's a date-time or time,
     * the format must correspond to the format specified
     * in the config.
     *
     * If it's a temporal expression, it can have one
     * of the following formats:
     *
     * * "30 seconds ago"
     * * "5 minutes ago"
     * * "1 hour ago"
     * * "6 days ago"
     * * "1 week ago"
     * * ...
     *
     * If the string is in an invalid format, returns 0.
     */
    time_t parse_temporal(const std::string& string);
    /**
     * Attempts to parse a relative time string in the
     * format of:
     *
     * * "30 seconds"
     * * "5 minutes"
     * * "1 hour"
     * * "6 days"
     * * "1 week"
     * * ...
     *
     * Returns 0 if the parse failed.
     */
    time_t parse_relative_time(const std::string& string);
}
