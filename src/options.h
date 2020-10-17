#include <memory>
#include "libs/cxxopts.hpp"

#pragma once

/**
 * Deals with the display and processing of command line options.
 */
namespace options {
    /// Processes the given command line options.
    void process(int& arg_count, char **&args);
}
