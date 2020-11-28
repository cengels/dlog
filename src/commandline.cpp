#include <iostream>
#include <stdexcept>
#include <string>
#include <memory>
#include "cxxopts.hpp"
#include "version.h"
#include "commandline.h"
#include "subcommands/subcommands.h"

/**
 * Processes any subcommands immediately following the program name
 * and removes the corresponding arguments from the args array.
 */
static std::string parse_subcommand(int& arg_count, char **& args)
{
    std::string subcommand = std::string();

    for (int i = 1; i < arg_count && args[i][0] != '-' && subcommand.empty(); i++) {
        subcommand += args[i];
        // "delete" array element by copying it to the end and decrementing arg_count
        std::copy(args+i+1, args+arg_count, args+i);
        arg_count--;
        i--;
    }

    return subcommand;
}

int command_line::parse(int& arg_count, char **&args)
{
    const auto subcommand_string = parse_subcommand(arg_count, args);
    auto* subcommand = subcommands::find(subcommand_string);

    if (subcommand == nullptr) {
        std::cout << "Invalid command: " << subcommand_string << std::endl
                  << "Run dlog -h to view a list of available commands." << std::endl;

        return 2;
    }

    const auto parsedOptions = subcommand->options().parse(arg_count, args);

    return subcommand->run(parsedOptions);
}

bool command_line::confirm()
{
    std::string input;

    return std::getline(std::cin, input) && (input == "y" || input == "Y");
}
