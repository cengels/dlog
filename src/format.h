#include <string>
#include <time.h>
#include "entries.h"

#pragma once

namespace format {
    /**
     * Formats a POSIX time point as a local time string
     * in the format of hours:minutes:seconds.
     */
    std::string as_local_time_string(const time_t& time);

    /**
     * Writes a local time string to the output stream.
     */
    std::ostream& local_time(std::ostream& stream, const time_t& time);

    /**
     * Returns a local date string as a string.
     */
    std::string date_string(const time_t& time);

    /**
     * Formats a POSIX time duration as a local duration string.
     */
    std::string as_duration(const time_t& duration, const bool& always_pad = false);

    /** Writes a formatted time entry to the output stream. */
    std::ostream& entry(std::ostream& stream, const entries::entry& entry);

    /** Writes a formatted time entry table to the output stream. */
    std::ostream& entries(std::ostream& stream, std::vector<entries::entry>::const_iterator begin, const std::vector<entries::entry>::const_iterator& end);

    /**
     * Attempts to convert a string to an int and returns a value indicating
     * whether the operation succeeded.
     */
    bool to_int(const std::string& string, int& result);
}
