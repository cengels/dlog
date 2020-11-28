#include <time.h>
#include <string>
#include <vector>
#include "entry.h"

#pragma once

namespace entries {
    /** Gets a copy of the last written time entry. */
    entries::entry last();

    /**
     * Appends a single entry to the back of the entries file
     * and returns a value indicating whether the operation was successful.
     */
    bool append(const entries::entry& entry);

    /**
     * Overwrites the nth entry from the back of the entries file
     * and returns a value indicating whether the operation was successful.
     *
     * Pass an uninitialized entry to delete the entry.
     */
    bool overwrite(uint which, const entries::entry& entry);

    /**
     * Removes the nth entry from the back of the entries file
     * and returns a value indicating whether the operation was successful.
     */
    bool remove(uint which);

    /**
     * Reads all entries from the back of the entries file and returns them.
     */
    std::vector<entries::entry> read_all(uint limit = 0);

    /**
     * Gets a specific entry from the end.
     */
    entries::entry get(uint which);
}
