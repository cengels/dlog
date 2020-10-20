#include <optional>
#include <experimental/filesystem>

#pragma once

namespace files {
    /**
     * Gets the directory path where dlog data is located according to the environment variable DLOG_PATH
     * or if that variable is not defined, a default path determined by the system.
     **/
    const std::optional<std::experimental::filesystem::path>& dlog_dir();

    /**
     * Gets the path of the specified file in the dlog directory.
     **/
    const std::optional<std::experimental::filesystem::path> dlog_file(const std::string& file_name);
}
