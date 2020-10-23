#include "subcommands.h"
#include "main.h"
#include "start.h"
#include "fill.h"

namespace {
    std::vector<subcommands::subcommand*>* vector = nullptr;
}

std::vector<subcommands::subcommand*>& subcommands::subcommands()
{
    if (vector == nullptr) {
        vector = new std::vector<subcommands::subcommand*>{
            new subcommands::main(),
            new subcommands::start(),
            new subcommands::fill()
        };
    }

    return *vector;
}

subcommands::subcommand* subcommands::find(const std::string& command)
{
    for (subcommands::subcommand* subcommand : subcommands::subcommands()) {
        if (subcommand->command() == command) {
            return subcommand;
        }
    }

    return nullptr;
}
