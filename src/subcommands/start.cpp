#include "../cli/color.h"
#include "version.h"
#include "../entries.h"
#include "../format.h"
#include "subcommands.h"
#include "start.h"

cxxopts::Options subcommands::start::options() const
{
    auto opts = subcommands::subcommand::options();

    opts.add_options()
        ("h,help", "Prints all available options.");

    return opts;
}

int subcommands::start::run(const cxxopts::ParseResult& parsedOptions)
{
    if (parsedOptions.count("help") != 0) {
        return subcommands::subcommand::run(parsedOptions);
    }

    const auto last = entries::last();

    if (last.valid() && !last.complete()) {
        std::cout << "You still have an unstopped time entry started at "
                  << cli::color::time << format::as_local_time_string(last.from) << cli::color::reset
                  << ".\nUse " << cli::color::command << "dlog fill" << cli::color::reset
                  << " to stop the last time entry first."
                  << std::endl;

        return 1;
    }

    entries::entry entry;
    entry.from = std::time(nullptr);

    if (!entries::write(entry)) {
        std::cerr << cli::color::error << "Error: " << cli::color::error_message << "Couldn't write to entries file.\n"
                     "Do you have write permission?"
                  << cli::color::reset << std::endl;
        return 1;
    }

    std::cout << "Started a new incomplete time entry at "
              << cli::color::time << format::as_local_time_string(entry.from) << cli::color::reset
              << std::endl;

    return 0;
}

std::string subcommands::start::command() const
{
    return "start";
}

std::string subcommands::start::syntax() const
{
    return "[-h]";
}

std::string subcommands::start::description() const
{
    return "Starts a new unnamed time entry. Call dlog fill to stop it.";
}
