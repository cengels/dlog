#include <string>

#pragma once

struct config
{
    /** Path to the directory where the time entries and other information is stored. */
    std::string output_directory;
    /** If true, any time when a new activity, project, or tag were to be created,
     * asks the user for confirmation first. This can prevent typos from accidentally
     * creating new items.
     */
    bool confirm_new;
};
