#include <utility>
#include "subcommand.h"

int subcommands::subcommand::run(const cxxopts::ParseResult& parsedOptions)
{
    if (parsedOptions.count("help") > 0 || parsedOptions.arguments().empty()) {
        std::cout << this->help() << std::endl;
    }

    return 0;
}

cxxopts::Options subcommands::subcommand::options() const
{
    std::string command = this->command();

    if (command.empty()) {
        command.append("dlog");
    } else {
        command.insert(0, "dlog ");
    }

    cxxopts::Options opts(command);

    std::string custom_help = this->syntax();
    custom_help.append("\n\n  ");
    custom_help.append(this->description());

    opts.custom_help(custom_help);

    return opts;
}

std::string subcommands::subcommand::help() const
{
    return this->options().help();
}

std::string subcommands::subcommand::syntax() const
{
    return std::string();
}
