#include <iostream>
#include <memory>
#include <string>
#include "libs/cxxopts.hpp"
#include "./options.h"

int main(int arg_count, char *args[]) {
    const auto options = options::get_options();

    std::cout << "running dlog" << std::endl;

    try {
        const cxxopts::ParseResult result = options->parse(arg_count, args);

        options::process(result, *options);
    } catch (const cxxopts::OptionException& e) {
        std::cout << e.what() << std::endl;

        return 1;
    }

    return 0;
}
