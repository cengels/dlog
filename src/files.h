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
     * Creates an empty temporary (.tmp) file from the file specified by path
     * and returns this path if successfully copied.
     *
     * This function returns an empty path if no temp file could be created.
     */
    std::experimental::filesystem::path prepare_for_write(const std::experimental::filesystem::path& path, bool copy_contents = false);

    /** Restores the file specified by path from the .tmp file created by backup(). */
    bool restore(const std::experimental::filesystem::path& path);

    /** Accepts changes made to a file previously prepared for write by removing the associated .tmp file. */
    bool accept_changes(const std::experimental::filesystem::path& path);

    /**
     * Writes the last line of the specified file stream to result
     * and returns a value indicating whether the beginning was reached.
     */
    bool get_last_line(std::istream& stream, std::string& result);

    /**
     * Writes the last non-empty line of the specified file stream to result
     * and returns a value indicating whether the beginning was reached.
     *
     * If all lines are empty, result will also be empty.
     */
    bool get_last_nonempty_line(std::istream& stream, std::string& result);

    /**
     * Writes the previous line in the specified file stream
     * into result and returns a value indicating whether the beginning
     * was reached.
     *
     * If the stream is positioned at the beginning,
     * positions the stream at the end and writes the last line instead.
     */
    bool get_previous_line(std::istream& stream, std::string& result);

    /**
     * Sets the stream position to the beginning of the first empty line
     * starting from the end of the file. This places the stream in a
     * state where any subsequent writes will append to the end of the file
     * without overwriting any meaningful content.
     *
     * If the file does not end in a blank line, this function inserts one,
     * thus guaranteeing that subsequent writes do not write to the end
     * of a line with existing content.
     *
     * If there are multiple blank lines at the end of the file, this function
     * sets the stream position to the beginning of the first of them. This
     * may result in the remaining blank lines being overwritten by subsequent
     * writes.
     *
     * For this function to work, the file stream has to be opened using both
     * the in and out flags.
     */
    void append_to_last_line(std::fstream& stream);

    /** Opens the specified path in the default system pager. */
    bool open_in_pager(const std::experimental::filesystem::path& path);
}
