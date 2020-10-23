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

    return opts;
}

int subcommands::fill::run(const cxxopts::ParseResult& parsedOptions)
{
    if (parsedOptions.count("help") != 0) {
        return subcommands::subcommand::run(parsedOptions);
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
