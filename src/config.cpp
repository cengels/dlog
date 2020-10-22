#include <fstream>
#include <sstream>
#include <iostream>
#include "config.h"
#include "files.h"

namespace {
    std::optional<config::structure> m_config;
}

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

    if (path.empty()) {
        return structure;
    }

    std::ifstream in(path.c_str());

    if (in.good()) {
        parse_config(in, structure);
    }

    in.close();

    return structure;
}

static void save_config()
{
}

bool config::available()
{
    return !config::path().empty();
}

const config::structure& config::config()
{
    if (!m_config.has_value()) {
        m_config = load_config_file();
    }

    return *m_config;
}

const std::experimental::filesystem::path config::path()
{
    return files::dlog_file("dlog.config");
}

void config::set(const std::string& property, const std::string& value)
{
}
