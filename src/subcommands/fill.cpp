#include "rang.hpp"
#include "version.h"
#include "../entries.h"
#include "../format.h"
#include "subcommands.h"
#include "fill.h"

cxxopts::Options subcommands::fill::options() const
{
    auto opts = subcommands::subcommand::options();

    opts.add_options()
        ("h,help", "Prints all available options.");

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
            std::cout << "Invalid input: \"";
            format::colorize::activity(std::cout, positional);
            std::cout << "\"\nCommas and quotes are not allowed.";

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
        return subcommands::subcommand::run(parsedOptions);
    }

    entries::entry entry = parse_entry(positionals);

    const entries::entry& last = entries::last();

    if (!last.valid()) {
        std::cout << "No entries found. There must be at least one entry before\n"
                     "you can call ";
        format::colorize::command(std::cout, "dlog fill");
        std::cout << ". Call ";
        format::colorize::command(std::cout, "dlog start");
        std::cout << " to create one.";

        return 2;
    }

    if (last.complete()) {
        // append new entry
        entry.from = last.to;

        if (!entry.valid()) {
            return 1;
        }

        entries::write(entry);

        std::cout << "Filled ";
        format::entry(std::cout, entry);
        std::cout << "." << std::endl;
    } else {
        // overwrite last entry
        entry.from = last.from;

        if (!entry.valid()) {
            return 1;
        }

        std::cout << "Filled ";
        format::entry(std::cout, entry);
        std::cout << "." << std::endl;
    }

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
