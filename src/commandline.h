#pragma once

/**
 * Deals with the display and processing of command line arguments.
 */
namespace command_line {
    /** Parses the given command line arguments. */
    int parse(int& arg_count, char **&args);
    /** Asks the user for confirmation and returns their response. */
    bool confirm();
}
