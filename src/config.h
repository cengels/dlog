#include <string>
#include <optional>
#include <experimental/filesystem>

#pragma once

/** Deals with retrieving, storing, and setting config properties. */
namespace config {
    struct structure
    {
        /**
         * If true, any time when a new activity, project, or tag were to be created,
         * asks the user for confirmation first. This can prevent typos from accidentally
         * creating new items.
         */
        bool confirm_new = false;
    };

    /** Gets the config, loading it from the file system if it hasn't been already. */
    const config::structure& config();

    /**
     * Gets the file path where the config is located according to the environment variable DLOG_PATH
     * or if that variable is not defined, a default path determined by the system.
     *
     * If no path can be found at all, this function returns an empty path.
     */
    const std::experimental::filesystem::path path();

    /** Sets a config property and writes it to the file system. */
    void set(const std::string& property, const std::string& value);
}
