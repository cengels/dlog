#include "start.h"
#include "version.h"
#include "../entries.h"
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
    auto last = entries::last();
    std::cout << "from: [" << last.from << "]\n"
              "to: [" << last.to << "]\n"
              "activity: [" << last.activity << "]\n"
              "project: [" << last.project << "]\n";

    for (const auto& tag : last.tags) {
        std::cout << "[" << tag << "]";
    }

    std::cout << std::endl;

    // Prints help
    // return subcommands::subcommand::run(parsedOptions);
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
