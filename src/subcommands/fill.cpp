#include <set>
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
        ("d,duration", "A duration to say how long the entry lasted from start.", cxxopts::value<std::string>())
        ("m,message", "A comment to be added to the entry.", cxxopts::value<std::string>())
        ("u,update", "Updates the last entry instead of adding a new one.");

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
    std::set<std::string> tags;
    std::string tag;

    for (const auto& positional : positionals) {
        if (!is_valid(positional)) {
            std::cout << "Invalid input: \""
                      << cli::color::activity << positional << cli::color::reset
                      << "\"\nCommas and quotes are not allowed." << std::endl;

            return entries::entry();
        }

        if (positional.at(0) == '+') {
            if (!tag.empty()) {
                tags.insert(tag);
                tag.clear();
            }

            tag = positional.substr(1);
        } else if (!tags.empty() || !tag.empty()) {
            tag.append(" ");
            tag.append(positional);
        } else {
            if (!activity.empty()) {
                activity.append(" ");
            }

            activity.append(positional);
        }
    }

    if (!tag.empty()) {
        tags.insert(tag);
    }

    const int colon_position = activity.find(':', 1);

    if (colon_position == std::string::npos) {
        entry.activity = activity;
    } else {
        entry.activity = activity.substr(0, colon_position);
        entry.project = activity.substr(colon_position + 1);
    }

    entry.tags = std::vector<std::string>(tags.begin(), tags.end());

    return entry;
}

static bool parse_from(const cxxopts::ParseResult& parsedOptions, entries::entry& entry)
{
    if (parsedOptions.count("from") > 0) {
        int from = parser::parse_temporal(parsedOptions["from"].as<std::string>());

        if (from == 0) {
            std::cout << "Invalid format in \"from\". Use -h for help." << std::endl;

            return false;
        }

        entry.from = from;
    }

    return true;
}

static bool parse_to(const cxxopts::ParseResult& parsedOptions, entries::entry& entry)
{
    if (parsedOptions.count("to") > 0) {
        int to = parser::parse_temporal(parsedOptions["to"].as<std::string>());

        if (to == 0) {
            std::cout << "Invalid format in \"to\". Invalid format. Use -h for help." << std::endl;

            return false;
        }

        entry.to = to;

        if (parsedOptions.count("duration") > 0) {
            std::cout << "Invalid format in \"duration\". Cannot specify both \"to\" and \"duration\"." << std::endl;

            return false;
        }
    }

    return true;
}

static bool parse_duration(const cxxopts::ParseResult& parsedOptions, entries::entry& entry)
{
    if (parsedOptions.count("duration") > 0) {
        int duration = parser::parse_relative_time(parsedOptions["duration"].as<std::string>());

        if (duration == 0) {
            std::cout << "Invalid option: \"duration\". Invalid format. Use -h for help." << std::endl;

            return false;
        }

        entry.to = entry.from + duration;
    }

    return true;
}

static int run_update(const cxxopts::ParseResult& parsedOptions)
{
    const auto& positionals = parsedOptions.unmatched();

    if (!positionals.empty()
        || parsedOptions.count("from") > 0) {
        std::cout << "When using -u, only -d and -t are valid." << std::endl;

        return 1;
    }

    entries::entry last = entries::last();

    if (!last.complete()) {
        std::cout << "The last entry must be a completed entry, meaning you have to use "
                  << cli::color::command << "dlog fill" << cli::color::reset
                  << " without the -u option at least once." << std::endl;

        return 1;
    }

    last.to = std::time(nullptr);

    if (!parse_to(parsedOptions, last)) {
        return 1;
    }

    if (!parse_duration(parsedOptions, last)) {
        return 1;
    }

    if (!last.valid()) {
        std::cout << "A time entry cannot be in the future." << std::endl;

        return 1;
    }

    entries::overwrite(0, last);

    std::cout << "Updated ";
    format::entry(std::cout, last);
    std::cout << "." << std::endl;

    return 0;
}

int subcommands::fill::run(const cxxopts::ParseResult& parsedOptions)
{
    if (parsedOptions.count("help") != 0) {
        return subcommands::subcommand::run(parsedOptions);
    }

    const auto& positionals = parsedOptions.unmatched();

    if (parsedOptions.count("update") > 0) {
        return run_update(parsedOptions);
    }

    entries::entry entry = parse_entry(positionals);

    if (parsedOptions.count("message") > 0) {
        entry.comment = parsedOptions["message"].as<std::string>();
    }

    const entries::entry& last = entries::last();

    if (!last.valid()) {
        std::cout << "No entries found. There must be at least one entry before\n"
                     "you can call "
                  << cli::color::command << "dlog fill" << cli::color::reset
                  << ". Call "
                  << cli::color::command << "dlog start" << cli::color::reset
                  << " to create one." << std::endl;

        return 1;
    }

    if (last.complete()) {
        // append new entry
        entry.from = last.to;
    } else {
        // overwrite last entry
        entry.from = last.from;
    }

    if (!parse_from(parsedOptions, entry)) {
        return 1;
    }

    if (!parse_to(parsedOptions, entry)) {
        return 1;
    }

    if (!parse_duration(parsedOptions, entry)) {
        return 1;
    }

    if (!entry.valid()) {
        std::cout << "A time entry cannot be in the future." << std::endl;

        return 1;
    }

    if (!entry.complete()) {
        std::cout << "No activity specified. Use "
                  << cli::color::command << "dlog fill -h" << cli::color::reset
                  << " for usage syntax." << std::endl;
        return 1;
    }

    if (last.complete()) {
        entries::append(entry);
    } else {
        entries::overwrite(0, entry);
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
    return "[-h] <activity> [+<tag>...] [-m <message>] [-u] [-f <datetime>] [-t <datetime>]\n"
           "            [-h] <activity>:<project> [+<tag>...] [-m <message>] [-u] [-f <datetime>] [-t <datetime>]";
}

std::string subcommands::fill::description() const
{
    return "Fills the time between the last time entry and now with a new time entry.\n"
           "  Activities, projects, and tags all allow whitespace within them, but not quotes or commas.\n\n"
           "  Examples:\n"
           "    dlog fill watch:yt +my-favorite-channel\n"
           "    dlog fill gaming:cyberpunk 2077 +singleplayer +pc -t \"1 hour ago\"\n"
           "    dlog fill shopping:online -m \"new headphones\"\n"
           "    dlog fill shower";
}
