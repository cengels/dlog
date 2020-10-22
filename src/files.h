#include <optional>
#include <experimental/filesystem>

#pragma once

namespace files {
    /**
     * Gets the directory path where dlog data is located according to the environment variable DLOG_PATH
     * or if that variable is not defined, a default path determined by the system.
     *
     * In some cases, a dlog directory may not be able to be determined. In that case, this function
     * will write appropriate error output to cerr and return an empty path.
     */
    const std::experimental::filesystem::path& dlog_dir();

    /**
     * Gets the path of the specified file in the dlog directory.
     *
     * If no directory for dlog can be determined, this function returns an empty path.
     */
    const std::experimental::filesystem::path dlog_file(const std::string& file_name);
}
