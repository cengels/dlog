#include "version.h"
#include "../cli/color.h"
#include "../entries.h"
#include "../format.h"
#include "../parser.h"
#include "../commandline.h"
#include "subcommands.h"
#include "remove.h"

cxxopts::Options subcommands::remove::options() const
{
    auto opts = subcommands::subcommand::options();

    opts.add_options()
        ("h,help", "Prints all available options.")
        ("n", "If specified, removes the last nth entry.", cxxopts::value<int>())
        ("y,yes", "Suppresses the confirmation prompt before deletion.");

    return opts;
}

int subcommands::remove::run(const cxxopts::ParseResult& parsedOptions)
{
    if (parsedOptions.count("help") != 0) {
        return subcommands::subcommand::run(parsedOptions);
    }

    int which = 1;
    const bool suppress_confirmation = parsedOptions.count("y") != 0;

    if (parsedOptions.count("n") != 0) {
        which = parsedOptions["n"].as<int>();

        if (which <= 0) {
            std::cout << "n must be a number above 0. Use -h for help." << std::endl;

            return 1;
        }
    }

    entries::entry entry = entries::get(which);

    if (!entry.valid()) {
        std::cout << "There was no entry at position " << which << "." << std::endl;

        return 1;
    }

    if (!suppress_confirmation) {
        std::cout << "Are you sure you want to delete entry ";
        format::entry(std::cout, entry);
        std::cout << "? [y/n] ";

        if (!command_line::confirm()) {
            std::cout << "Aborting." << std::endl;
            return 0;
        }
    }

    if (entries::remove(which - 1)) {
        std::cout << "Entry removed." << std::endl;
        return 1;
    }

    std::cout << "Could not remove entry due to an unknown error." << std::endl;

    return 0;
}

std::string subcommands::remove::command() const
{
    return "remove";
}

std::string subcommands::remove::syntax() const
{
    return "[-h] [-n <integer>] [-y]";
}

std::string subcommands::remove::description() const
{
    return "Removes the last time entry or the last nth time entry.\n"
           "  Asks for confirmation before deletion.\n\n"
           "  Examples:\n"
           "    dlog remove\n"
           "    dlog remove -n 2";
}
