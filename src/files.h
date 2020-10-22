#include <optional>
#include <experimental/filesystem>
#include <fstream>

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

    /**
     * Creates a temporary (.tmp) file with the same contents as the file specified by path
     * and returns true if successfully copied.
     *
     * Note that this function returns true even if the file does not exist. In that case,
     * no .tmp file is created and a restore() simply deletes the newly created file.
     *
     * If this function returns false, this means that the file already existed but no .tmp
     * file could be created for it. In that case, you should always abort subsequent writes.
     */
    bool prepare_for_write(const std::experimental::filesystem::path& path);

    /** Restores the file specified by path from the .tmp file created by backup(). */
    void restore(const std::experimental::filesystem::path& path);

    /** Accepts changes made to a file previously prepared for write by removing the associated .tmp file. */
    void accept_changes(const std::experimental::filesystem::path& path);

    /**
     * Gets the last line of the specified file stream
     * or an empty string if anything goes wrong.
     */
    std::string get_last_line(std::ifstream& stream);
}
