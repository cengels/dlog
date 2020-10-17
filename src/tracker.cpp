#include <iostream>
#include <string>
#include "tracker.h"

void tracker::handle_new_foreground_process(const std::string& process_path) {
    std::cout << "New process path is " << process_path << std::endl;
}
