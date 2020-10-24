#include <time.h>
#include <string>
#include <vector>

#pragma once

namespace entries {
    /** Represents a dlog time entry. */
    struct entry
    {
        entry();

        /** A POSIX time point defining the start of the time entry. */
        time_t from;
        /** A POSIX time point defining the end of the time entry. */
        time_t to;
        /**
         * The activity committed during this time entry.
         * Must not be an empty string unless the time entry is incomplete.
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

        /** Returns true if this time entry is valid. */
        bool valid() const;

        /**
         * Returns true if this time entry is complete, that is
         * if it has both a start and an end point.
         */
        bool complete() const;
    };

    /** Gets a copy of the last written time entry. */
    void serialize(std::ostream& stream, const entries::entry& entry);

    /** Gets a copy of the last written time entry. */
    entries::entry last();

    /**
     * Appends a single entry to the back of the entries file
     * and returns a value indicating whether the operation was successful.
     */
    bool write(const entries::entry& entry);

    /**
     * Overwrites the last entry in the entries file
     * and returns a value indicating whether the operation was successful.
     */
    bool overwrite_last(const entries::entry& entry);

    /**
     * Reads all entries from the back of the entries file and returns them.
     */
    std::vector<entries::entry> read_all(uint limit = 0);
}
