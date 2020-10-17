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

static cxxopts::Options get_options() {
    auto options = cxxopts::Options("dlog", "Configurable command line time tracking.");

    options.add_options()
        ("v,version", "Prints the installed version.")
        ("h,help", "Prints all available commands.")
        ("f,file", "Specify an output file.", cxxopts::value<std::string>());

    return options;
}

static void process_options(const cxxopts::ParseResult& parse_result, const cxxopts::Options& options) {
    if (parse_result.count("subcommand") > 0) {
        std::cout << parse_result["subcommand"].as<std::string>();
    }

    if (parse_result.count("version") > 0) {
        print_version();
    }

    if (parse_result.count("help") > 0 || parse_result.arguments().empty()) {
        std::cout << options.help() << std::endl;
    }
}

/**
 * Processes any subcommands immediately following the program name
 * and removes the corresponding arguments from the args array.
 */
static std::string process_subcommand(int& arg_count, char **& args) {
    std::string subcommand = std::string();

    for (int i = 1; i < arg_count && args[i][0] != '-'; i++) {
        if (!subcommand.empty()) {
            subcommand += " ";
        }

        subcommand += args[i];
        // "delete" array element by copying it to the end and decrementing arg_count
        std::copy(args+i+1, args+arg_count, args+i);
        arg_count--;
        i--;
    }

    return subcommand;
}

void options::process(int& arg_count, char **&args) {
    const auto subcommand = process_subcommand(arg_count, args);

    cxxopts::Options options = get_options();
    const cxxopts::ParseResult result = options.parse(arg_count, args);

    process_options(result, options);
}
