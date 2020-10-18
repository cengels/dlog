#include "start.h"
#include "version.h"
#include "subcommands.h"

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
    return subcommands::subcommand::run(parsedOptions);
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
