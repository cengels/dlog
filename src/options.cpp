#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include "./version.h"
#include "libs/cxxopts.hpp"
#include "./options.h"
#include "window_hooks/window_hooks.h"

void print_version() {
    std::cout << "dlog version "
        << dlog_VERSION_MAJOR << "."
        << dlog_VERSION_MINOR << "."
        << dlog_VERSION_PATCH << std::endl;
}

void print_foreground_window_debug(const std::string& window_title, const std::string& process_name) {
    std::cout << "New foreground window: " << window_title << " (" << process_name << ")";
}

void options::process_options(const cxxopts::ParseResult& parse_result, const cxxopts::Options& options) {
    if (parse_result.count("version") > 0) {
        print_version();
    }

    if (parse_result.count("help") > 0 || parse_result.arguments().empty()) {
        std::cout << options.help() << std::endl;
    }

    if (parse_result.count("track") > 0) {
        // if (parse_results.count("file") == 0 || !std::filesystem::exists(&parse_result["file"])) {
        //     throw std::invalid_argument("No file or invalid file path specified. Make sure the file exists.");
        // }

        window_hooks::register_foreground_window_listener(&print_foreground_window_debug);
    }
}

std::unique_ptr<cxxopts::Options> options::get_options() {
    auto options = std::make_unique<cxxopts::Options>("dlog", "Configurable automated command line time tracking.");

    options->add_options()
        ("v,version", "Prints the installed version.")
        ("h,help", "Prints all available commands.")
        ("t,track", "Starts tracking the foreground window and writes it to the file specified with -f.")
        ("f,file", "Specify an output file.", cxxopts::value<std::string>());

    return options;
}
