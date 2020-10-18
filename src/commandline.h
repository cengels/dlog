#include <memory>
#include "libs/cxxopts.hpp"

#pragma once

/**
 * Deals with the display and processing of command line arguments.
 */
namespace command_line {
    /// Parses the given command line arguments.
    void parse(int& arg_count, char **&args);
}
