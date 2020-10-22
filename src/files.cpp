#include <iostream>
#include "files.h"

namespace {
    constexpr char ENV_VARIABLE[] = "DLOG_PATH";
    std::optional<std::experimental::filesystem::path> m_dlog_dir;
}

#ifdef IS_WINDOWS
static const const std::experimental::filesystem::path default_path()
{
    const char* app_data_env_variable = getenv("AppData");

    if (app_data_env_variable == nullptr) {
        std::cerr << "Error: Could not locate AppData directory. This is likely not your fault.\n"
                     "To fix this problem, try defining your own config path as the environment variable DLOG_PATH." << std::endl;
        return std::experimental::filesystem::path();
    }

    std::experimental::filesystem::path app_data_path(app_data_env_variable);
    app_data_path.append("Roaming");
    app_data_path.append("dlog");

    std::error_code error;
    if (!std::experimental::filesystem::create_directory(app_data_path, error) && error) {
        std::cerr << "Error: Could not create config directory.\n"
                     "File system reported: " << error.message() << " (" << error.value() << ")" << std::endl;
        return std::experimental::filesystem::path();
    }

    return app_data_path;
}
#else
static const std::experimental::filesystem::path default_path()
{
    std::experimental::filesystem::path config_path;
    char* home_env_variable = getenv("XDG_DATA_HOME");

    if (home_env_variable != nullptr) {
        config_path = home_env_variable;
    } else {
        home_env_variable = getenv("HOME");

        if (home_env_variable == nullptr) {
            std::cerr << "Error: Could not locate HOME directory. This is likely not your fault.\n"
                         "To fix this problem, try defining your own config path as the environment variable DLOG_PATH." << std::endl;
        return std::experimental::filesystem::path();
        }

        config_path = home_env_variable;
        config_path.append(".config");
        config_path.append("dlog");
    }

    std::error_code error;
    if (!std::experimental::filesystem::create_directories(config_path, error) && error) {
        std::cerr << "Error: Could not create config directory.\n"
                     "File system reported: " << error.message() << " (" << error.value() << ")" << std::endl;
        return std::experimental::filesystem::path();
    }

    return config_path;
}
#endif

const std::experimental::filesystem::path& files::dlog_dir()
{
    if (!m_dlog_dir.has_value()) {
        const auto path = getenv(static_cast<const char*>(ENV_VARIABLE));

        if (path != nullptr) {
            const std::experimental::filesystem::path custom_path(path);

            if (std::experimental::filesystem::exists(custom_path)) {
                m_dlog_dir = std::experimental::filesystem::path(path);
            } else {
                std::cerr << "Error: Invalid DLOG_PATH [" << path << "].\n"
                             "Check your DLOG_PATH environment variable and make sure it's a valid directory." << std::endl;
                m_dlog_dir = std::experimental::filesystem::path();
            }
        } else {
            m_dlog_dir = default_path();
        }
    }

    return *m_dlog_dir;
}

const std::experimental::filesystem::path files::dlog_file(const std::string& file_name)
{
    const auto& directory = files::dlog_dir();

    if (directory.empty()) {
        return directory;
    }

    auto file = std::experimental::filesystem::path(directory);
    file.append(file_name);

    return file;
}
