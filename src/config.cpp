#include <optional>
#include <fstream>
#include <sstream>
#include <iostream>
#include "config.h"

namespace {
    std::optional<config::structure> m_config;
    constexpr char ENV_VARIABLE[] = "DLOG_PATH";
}

#ifdef IS_WINDOWS
static const char* default_path()
{
    const char* app_data_env_variable = getenv("AppData");

    if (app_data_env_variable == nullptr) {
        std::cerr << "Error: Could not locate AppData directory. This is likely not your fault.\n"
                     "To fix this problem, try defining your own config path as the environment variable DLOG_PATH." << std::endl;
        return nullptr;
    }

    std::experimental::filesystem::path app_data_path(app_data_env_variable);
    app_data_path.append("Roaming");
    app_data_path.append("dlog");

    std::error_code error;
    if (!std::experimental::filesystem::create_directory(app_data_path, error) && error) {
        std::cerr << "Error: Could not create config directory.\n"
                     "File system reported: " << error.message() << " (" << error.value() << ")" << std::endl;
        return nullptr;
    }

    app_data_path.append("dlog.config");

    return app_data_path;
}
#else
static const std::optional<std::experimental::filesystem::path> default_path()
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
            return {};
        }

        config_path = home_env_variable;
        config_path.append(".config");
        config_path.append("dlog");
    }

    std::error_code error;
    if (!std::experimental::filesystem::create_directories(config_path, error) && error) {
        std::cerr << "Error: Could not create config directory.\n"
                     "File system reported: " << error.message() << " (" << error.value() << ")" << std::endl;
        return {};
    }

    config_path.append("dlog.config");

    return config_path;
}
#endif

static bool parse_property(std::istream& in, config::structure& structure, const std::string& property)
{
    if (property == "confirm_new") {
        bool confirm_new = false;
        in >> std::boolalpha >> confirm_new;

        if (in.fail()) {
            return false;
        }

        structure.confirm_new = confirm_new;
    }

    return true;
}

static void parse_config(std::istream& in, config::structure& structure)
{
    std::stringstream property_buffer;
    // States:
    // 0 -> no property parsed yet (start of line)
    // 1 -> property is not empty
    // 2 -> found end of property
    int state = 0;

    for (char c; in.get(c);) {
        if (c == '\n') {
            // End of line. Empty the property buffer.
            property_buffer.str(std::string());
            property_buffer.clear();
            state = 0;
        } else {
            if (c == ' ' || c == '=') {
                if (state == 1) {
                    state = 2;
                }
            } else {
                if (state <= 1) {
                    property_buffer << c;
                    state = 1;
                } else {
                    const std::string property = property_buffer.str();
                    in.putback(c);
                    parse_property(in, structure, property);
                }
            }
        }
    }
}

static config::structure load_config_file()
{
    config::structure structure;
    const auto path = config::path();

    if (!path.has_value()) {
        return structure;
    }

    std::ifstream in(path->c_str());

    if (in.fail()) {
        in.close();
        in.open(default_path()->c_str());
    }

    if (in.good()) {
        parse_config(in, structure);
    }

    in.close();

    return structure;
}

static void save_config()
{
}

const config::structure& config::config()
{
    if (!m_config.has_value()) {
        m_config = load_config_file();
    }

    return *m_config;
}

const std::optional<std::experimental::filesystem::path> config::path()
{
    const auto path = getenv(static_cast<const char*>(ENV_VARIABLE));

    if (path != nullptr) {
        std::experimental::filesystem::path config_path(path);
        config_path.append("dlog.config");

        return config_path;
    }

    return default_path();
}

void config::set(const std::string& property, const std::string& value)
{
}
