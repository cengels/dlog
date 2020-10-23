#include <iostream>
#include "files.h"
#include "rang.hpp"

namespace fs = std::experimental::filesystem;

namespace {
    constexpr char ENV_VARIABLE[] = "DLOG_PATH";
    std::optional<fs::path> m_dlog_dir;
}

#ifdef IS_WINDOWS
static const const fs::path default_path()
{
    const char* app_data_env_variable = getenv("AppData");

    if (app_data_env_variable == nullptr) {
        std::cerr << rang::fg::red << "Error: Could not locate AppData directory. This is likely not your fault.\n"
                     "To fix this problem, try defining your own config path as the environment variable DLOG_PATH." << rang::fg::reset << std::endl;
        return fs::path();
    }

    fs::path app_data_path(app_data_env_variable);
    app_data_path.append("Roaming");
    app_data_path.append("dlog");

    std::error_code error;
    if (!fs::create_directory(app_data_path, error) && error) {
        std::cerr << rang::fg::red << "Error: Could not create config directory.\n"
                     "File system reported: " << error.message() << " (" << error.value() << ")" << rang::fg::reset << std::endl;
        return fs::path();
    }

    return app_data_path;
}
#else
static const fs::path default_path()
{
    fs::path config_path;
    char* home_env_variable = getenv("XDG_DATA_HOME");

    if (home_env_variable != nullptr) {
        config_path = home_env_variable;
    } else {
        home_env_variable = getenv("HOME");

        if (home_env_variable == nullptr) {
            std::cerr << rang::fg::red << "Error: Could not locate HOME directory. This is likely not your fault.\n"
                         "To fix this problem, try defining your own config path as the environment variable DLOG_PATH." << rang::fg::reset << std::endl;
        return fs::path();
        }

        config_path = home_env_variable;
        config_path.append(".config");
        config_path.append("dlog");
    }

    std::error_code error;
    if (!fs::create_directories(config_path, error) && error) {
        std::cerr << rang::fg::red << "Error: Could not create config directory.\n"
                     "File system reported: " << error.message() << " (" << error.value() << ")" << rang::fg::reset << std::endl;
        return fs::path();
    }

    return config_path;
}
#endif

const fs::path& files::dlog_dir()
{
    if (!m_dlog_dir.has_value()) {
        const auto path = getenv(static_cast<const char*>(ENV_VARIABLE));

        if (path != nullptr) {
            const fs::path custom_path(path);

            if (fs::exists(custom_path)) {
                m_dlog_dir = fs::path(path);
            } else {
                std::cerr << rang::fg::red << "Error: Invalid DLOG_PATH [" << path << "].\n"
                             "Check your DLOG_PATH environment variable and make sure it's a valid directory." << rang::fg::reset << std::endl;
                m_dlog_dir = fs::path();
            }
        } else {
            m_dlog_dir = default_path();
        }
    }

    return *m_dlog_dir;
}

const fs::path files::dlog_file(const std::string& file_name)
{
    const auto& directory = files::dlog_dir();

    if (directory.empty()) {
        return directory;
    }

    auto file = fs::path(directory);
    file.append(file_name);

    return file;
}

bool files::prepare_for_write(const fs::path& path)
{
    if (!fs::exists(path)) {
        return true;
    }

    fs::path tmp(path);
    tmp.concat(".tmp");

    return fs::copy_file(path, tmp, fs::copy_options::overwrite_existing);
}

void files::accept_changes(const fs::path& path)
{
    fs::path tmp(path);
    tmp.concat(".tmp");

    fs::remove(tmp);
}

void files::restore(const fs::path& path)
{
    fs::path tmp(path);
    tmp.concat(".tmp");

    if (fs::exists(tmp)) {
        fs::copy_file(tmp, path, fs::copy_options::overwrite_existing);
    } else {
        // No .tmp file means the file did not exist prior to the write
        // so we can just delete it to go back to the previous state of the file system.
        fs::remove(path);
    }
}

bool files::get_last_line(std::istream& stream, std::string& result)
{
    if (stream.fail()) {
        return false;
    }

    stream.seekg(0, std::ios_base::end);

    if (stream.tellg() == 0) {
        // File is empty
        result.erase();
        return false;
    }

    return files::get_previous_line(stream, result);
}

bool files::get_last_nonempty_line(std::istream& stream, std::string& result)
{
    if (stream.fail()) {
        return false;
    }

    stream.seekg(0, std::ios_base::beg);

    while (files::get_previous_line(stream, result)) {
        if (!result.empty()) {
            return true;
        }
    }

    return false;
}

bool files::get_previous_line(std::istream& stream, std::string& result)
{
    if (stream.fail()) {
        return false;
    }

    if (stream.tellg() == 0) {
        return files::get_last_line(stream, result);
    }

    char character;

    result.erase();

    while (true) {
        stream.seekg(-1, std::ios_base::cur);

        if (stream.tellg() <= 0) {
            // Reached beginning of file
            break;
        }

        character = stream.peek();

        if (character != '\n') {
            result.insert(0, 1, character);
        } else {
            if (stream.tellg() <= 0) {
                // Reached beginning of file
                break;
            }

            stream.seekg(-1, std::ios_base::cur);
            character = stream.peek();

            if (character != '\r') {
                stream.seekg(1, std::ios_base::cur);
            }

            break;
        }
    }

    if (stream.tellg() <= 0) {
        stream.seekg(0);
        return false;
    }

    return true;
}

void files::append_to_last_line(std::fstream& stream)
{
    std::string line;
    while (true) {
        bool beginning_reached = !files::get_previous_line(stream, line);

        if (!line.empty()) {
            // Found non-empty line, go to end of line and insert new line there
            char c;
            int previous_position;

            // Because files::get_previous_line() leaves the
            // stream position at the end of the line before the line written to result,
            // we need to manually go back to the beginning of the resulting line first.

            stream.get(c);

            if (c == '\r') {
                stream.get(c);
            } else if (c != '\n') {
                stream.seekg(-1, std::ios_base::cur);
            }

            // After that, we iterate through that line again until we find either
            // the end of file or the next newline token.

            do {
                previous_position = stream.tellg();
                stream.get(c);
            } while (!stream.eof() && c != '\n');

            if (stream.eof()) {
                stream.clear();
                stream.seekg(previous_position, std::ios_base::beg);
                stream << "\n";
            }

            break;
        }

        if (beginning_reached) {
            // File is empty, seek position should remain 0
            break;
        }
    }
}
