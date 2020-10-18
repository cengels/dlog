#include <iostream>
#include <memory>
#include <string>
#include "libs/cxxopts.hpp"
#include "./commandline.h"

int main(int arg_count, char *args[]) {
    try {
        command_line::parse(arg_count, args);
    } catch (const cxxopts::OptionException& e) {
        std::cout << e.what() << std::endl;

        return 1;
    }

    return 0;
}
