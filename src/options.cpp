#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include "./version.h"
#include "libs/cxxopts.hpp"
#include "./options.h"

static void print_version() {
    std::cout << "dlog version "
        << dlog_VERSION_MAJOR << "."
        << dlog_VERSION_MINOR << "."
        << dlog_VERSION_PATCH << std::endl;
}

void options::process(const cxxopts::ParseResult& parse_result, const cxxopts::Options& options) {
    if (parse_result.count("version") > 0) {
        print_version();
    }

    if (parse_result.count("help") > 0 || parse_result.arguments().empty()) {
        std::cout << options.help() << std::endl;
    }
}

std::unique_ptr<cxxopts::Options> options::get_options() {
    auto options = std::make_unique<cxxopts::Options>("dlog", "Configurable command line time tracking.");

    options->add_options()
        ("v,version", "Prints the installed version.")
        ("h,help", "Prints all available commands.")
        ("f,file", "Specify an output file.", cxxopts::value<std::string>());

    return options;
}