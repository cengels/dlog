#include "main.h"
#include "version.h"
#include "subcommands.h"

static void print_version()
{
    std::cout << "dlog version "
        << dlog_VERSION_MAJOR << "."
        << dlog_VERSION_MINOR << "."
        << dlog_VERSION_PATCH << std::endl;
}

cxxopts::Options subcommands::main::options() const
{
    auto opts = subcommands::subcommand::options();

    opts.add_options()
        ("v,version", "Prints the installed version.")
        ("h,help", "Prints all available commands.");

    opts.custom_help("[-v | -h]");

    return opts;
}

int subcommands::main::run(const cxxopts::ParseResult& parsedOptions)
{
    if (parsedOptions.count("version") > 0) {
        print_version();
    }

    return subcommands::subcommand::run(parsedOptions);
}

std::string subcommands::main::command() const
{
    return std::string();
}

std::string subcommands::main::description() const
{
    return "Configurable command line time tracking.";
}

std::string subcommands::main::help() const
{
    auto string = subcommands::subcommand::help();

    string.append("\n"
    "  Commands:"
    "\n\n");

    for (const auto* subcommand : subcommands::subcommands()) {
        if (subcommand != this) {
            string.append("    dlog ");
            string.append(subcommand->command());
            string.append("\t");
            string.append(subcommand->description());
        }
    }

    return string;
}
