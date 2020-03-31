#include <iostream>
#include <memory>
#include "libs/cxxopts.hpp"
#include "./options.h"

int main(int arg_count, char *args[]) {
    const auto options = options::get_options();

    try {
        const cxxopts::ParseResult result = options->parse(arg_count, args);

        options::process_options(result, *options);
    } catch (const cxxopts::OptionException& e) {
        std::cout << e.what() << std::endl;

        return 1;
    }

    return 0;
}
