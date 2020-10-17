#include <memory>
#include "libs/cxxopts.hpp"

#pragma once

/**
 * Deals with the display and processing of command line options.
 */
namespace options {
    /// Processes the given command line options.
    void process(const cxxopts::ParseResult& parse_result, const cxxopts::Options& options);
    /// Gets all available command line options.
    std::unique_ptr<cxxopts::Options> get_options();
}
