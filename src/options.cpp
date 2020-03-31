#include <iostream>
#include <memory>
#include "version.h"
#include "libs/cxxopts.hpp"
#include "options.h"

void print_version() {
    std::cout << "dlog version "
        << dlog_VERSION_MAJOR << "."
        << dlog_VERSION_MINOR << "."
        << dlog_VERSION_PATCH << std::endl;
}

void options::process_options(const cxxopts::ParseResult& parse_result, const cxxopts::Options& options) {
    if (parse_result.count("version")) {
        print_version();
    }

    if (parse_result.count("help") || parse_result.arguments().size() == 0) {
        std::cout << options.help() << std::endl;
    }
}

std::unique_ptr<cxxopts::Options> options::get_options() {
    auto options = std::make_unique<cxxopts::Options>("dlog", "Configurable automated command line time tracking.");

    options->add_options()
        ("v,version", "Prints the installed version.")
        ("h,help", "Prints all available commands.")
        ("f,file", "Specify an output file.", cxxopts::value<std::string>());

    return options;
}
