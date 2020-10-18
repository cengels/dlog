#include <vector>
#include "subcommand.h"

#pragma once

namespace subcommands {
    std::vector<subcommands::subcommand*>& subcommands();

    /**
     * Attempts to find a subcommand that matches the passed string command.
     */
    subcommands::subcommand* find(const std::string& command);
}
