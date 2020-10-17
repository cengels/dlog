#include <time.h>
#include <string>
#include <vector>

#pragma once

/// Represents a dlog time entry.
struct entry {
    /// A POSIX time point defining the start of the time entry.
    time_t from;
    /// A POSIX time point defining the end of the time entry.
    time_t to;
    /**
     * The activity committed during this time entry.
     * Must not be an empty string.
     */
    std::string activity;
    /**
     * The project the activity was committed in.
     * Can be an empty string to mean that the activity does not belong
     * to a project.
     */
    std::string project;
    /**
     * A variable number of tags that belong to this time entry.
     */
    std::vector<std::string> tags;
};
