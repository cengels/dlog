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

std::string subcommands::main::syntax() const
{
    return "[-v] [-h]";
}

std::string subcommands::main::description() const
{
    return "dlog is a command-line interface that helps you\n  "
           "track your time and where you spend it.";
}

std::string subcommands::main::help() const
{
    auto string = subcommands::subcommand::help();

    string.append("\n"
    "Commands:"
    "\n");

    const auto& last = subcommands::subcommands().end();
    for (const auto* subcommand : subcommands::subcommands()) {
        if (subcommand != this) {
            string.append("  ");
            std::string command = subcommand->command();
            command.resize(8, ' ');
            string.append(command);
            string.append(" ");
            const std::string description = subcommand->description();
            string.append(description.substr(0, description.find('\n', 1)));

            if (subcommand != (*last)) {
                string.append("\n");
            }
        }
    }

    return string;
}
