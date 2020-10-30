#include "version.h"
#include "../cli/color.h"
#include "../entries.h"
#include "../format.h"
#include "../parser.h"
#include "subcommands.h"
#include "fill.h"

cxxopts::Options subcommands::fill::options() const
{
    auto opts = subcommands::subcommand::options();

    opts.add_options()
        ("h,help", "Prints all available options.")
        ("f,from", "A date-time, time, or temporal expression when the entry started.", cxxopts::value<std::string>())
        ("t,to", "A date-time, time, or temporal expression when the entry stopped.", cxxopts::value<std::string>())
        ("d,duration", "A duration to say how long the entry lasted from start.", cxxopts::value<std::string>());

    opts.allow_unrecognised_options();

    return opts;
}

static bool is_valid(const std::string& entry_string)
{
    return entry_string.find('"') == std::string::npos
        && entry_string.find(',') == std::string::npos;
}

static entries::entry parse_entry(const cxxopts::PositionalList& positionals)
{
    entries::entry entry;
    entry.to = std::time(nullptr);

    std::string activity;
    std::vector<std::string> tags;

    for (const auto& positional : positionals) {
        if (!is_valid(positional)) {
            std::cout << "Invalid input: \""
                      << cli::color::activity << positional << cli::color::reset
                      << "\"\nCommas and quotes are not allowed." << std::endl;

            return entries::entry();
        }

        if (positional.at(0) == '+') {
            tags.push_back(positional.substr(1));
        } else if (tags.empty()) {
            if (!activity.empty()) {
                activity.append(" ");
            }

            activity.append(positional);
        } else {
            std::string& last = *(tags.end() - 1);
            last.append(" ");
            last.append(positional);
        }
    }

    const int colon_position = activity.find(':', 1);

    if (colon_position == std::string::npos) {
        entry.activity = activity;
    } else {
        entry.activity = activity.substr(0, colon_position);
        entry.project = activity.substr(colon_position + 1);
    }

    entry.tags = tags;

    return entry;
}

int subcommands::fill::run(const cxxopts::ParseResult& parsedOptions)
{
    if (parsedOptions.count("help") != 0) {
        return subcommands::subcommand::run(parsedOptions);
    }

    const auto& positionals = parsedOptions.unmatched();

    if (positionals.empty()) {
        std::cout << "No activity specified. Use "
                  << cli::color::command << "dlog fill -h" << cli::color::reset
                  << " for usage syntax." << std::endl;
        return 1;
    }

    entries::entry entry = parse_entry(positionals);

    const entries::entry& last = entries::last();

    if (!last.valid()) {
        std::cout << "No entries found. There must be at least one entry before\n"
                     "you can call "
                  << cli::color::command << "dlog fill" << cli::color::reset
                  << ". Call "
                  << cli::color::command << "dlog start" << cli::color::reset
                  << " to create one." << std::endl;

        return 2;
    }

    if (last.complete()) {
        // append new entry
        entry.from = last.to;
    } else {
        if (parsedOptions.count("from") > 0) {
            std::cout << "Can't use \"from\" when the last entry is incomplete." << std::endl;

            return 1;
        }

        // overwrite last entry
        entry.from = last.from;
    }

    if (parsedOptions.count("from") > 0) {
        int from = parser::parse_temporal(parsedOptions["from"].as<std::string>());

        if (from == 0) {
            std::cout << "Invalid format in \"from\". Use -h for help." << std::endl;

            return 1;
        }

        format::local_time(std::cout, from);
        std::cout << std::endl;

        entry.from = from;
    }

    if (parsedOptions.count("to") > 0) {
        int to = parser::parse_temporal(parsedOptions["to"].as<std::string>());

        if (to == 0) {
            std::cout << "Invalid format in \"to\". Invalid format. Use -h for help." << std::endl;

            return 1;
        }

        format::local_time(std::cout, to);
        std::cout << std::endl;

        entry.to = to;

        if (parsedOptions.count("duration") > 0) {
            std::cout << "Invalid format in \"duration\". Cannot specify both \"to\" and \"duration\"." << std::endl;

            return 1;
        }
    }

    if (parsedOptions.count("duration") > 0) {
        int duration = parser::parse_relative_time(parsedOptions["duration"].as<std::string>());

        if (duration == 0) {
            std::cout << "Invalid option: \"duration\". Invalid format. Use -h for help." << std::endl;

            return 1;
        }

        entry.to = entry.from + duration;
    }

    if (!entry.valid()) {
        std::cout << "A time entry cannot be in the future." << std::endl;

        return 1;
    }

    if (last.complete()) {
        entries::write(entry);
    } else {
        entries::overwrite_last(entry);
    }

    std::cout << "Filled ";
    format::entry(std::cout, entry);
    std::cout << "." << std::endl;

    return 0;
}

std::string subcommands::fill::command() const
{
    return "fill";
}

std::string subcommands::fill::syntax() const
{
    return "[-h] <activity> [+<tag>...]\n"
           "            [-h] <activity>:<project> [+<tag>...]";
}

std::string subcommands::fill::description() const
{
    return "Fills the time between the last time entry and now with a new time entry.\n"
           "  Activities, projects, and tags all allow whitespace within them, but not quotes or commas.\n\n"
           "  Examples:\n"
           "    dlog fill watch:yt +my-favorite-channel\n"
           "    dlog fill gaming:cyberpunk 2077 +singleplayer +pc\n"
           "    dlog fill shower";
}
