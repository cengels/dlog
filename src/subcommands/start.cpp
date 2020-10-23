#include "rang.hpp"
#include "version.h"
#include "../entries.h"
#include "../format.h"
#include "subcommands.h"
#include "start.h"

cxxopts::Options subcommands::start::options() const
{
    auto opts = subcommands::subcommand::options();

    // opts.add_options()
    //     ("v,version", "Prints the installed version.")
    //     ("h,help", "Prints all available commands.");

    // opts.custom_help("[-v | -h]");

    return opts;
}

int subcommands::start::run(const cxxopts::ParseResult& parsedOptions)
{
    if (parsedOptions.count("help") != 0) {
        return subcommands::subcommand::run(parsedOptions);
    }

    const auto last = entries::last();

    if (last.valid() && !last.complete()) {
        std::cout << "You still have an unstopped time entry!\n"
                     "Use dlog fill to stop the last time entry first."
                  << std::endl;

        return 1;
    }

    entries::entry entry;
    entry.from = std::time(nullptr);

    if (!entries::write(entry)) {
        std::cerr << rang::fg::red << "Error: Couldn't write to entries file.\n"
                     "Please check that you have write permission."
                  << rang::fg::reset << std::endl;
        return 1;
    }

    std::cout << "Started a new incomplete time entry at "
              << format::as_local_time_string(entry.from)
              << std::endl;

    return 0;
}

std::string subcommands::start::command() const
{
    return "start";
}

std::string subcommands::start::syntax() const
{
    return std::string();
}

std::string subcommands::start::description() const
{
    return "Starts a new unnamed time entry. Call dlog fill to stop it.";
}
