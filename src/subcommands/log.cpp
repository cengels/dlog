#include <experimental/filesystem>
#include <ostream>
#include <fstream>
#include "rang.hpp"
#include "version.h"
#include "../entries.h"
#include "../format.h"
#include "../files.h"
#include "subcommands.h"
#include "log.h"

cxxopts::Options subcommands::log::options() const
{
    auto opts = subcommands::subcommand::options();

    opts.add_options()
        ("h,help", "Prints all available options.")
        ("P,no-pager", "Forces the log to print to standard output.")
        ("l,limit", "Limits the output to the last n entries.", cxxopts::value<int>());

    opts.allow_unrecognised_options();

    return opts;
}

int subcommands::log::run(const cxxopts::ParseResult& parsedOptions)
{
    if (parsedOptions.count("help") != 0) {
        return subcommands::subcommand::run(parsedOptions);
    }

    const auto& positionals = parsedOptions.unmatched();
    int limit = 0;

    if (parsedOptions.count("limit") != 0) {
        limit = parsedOptions["limit"].as<int>();
    }

    const auto& entries = entries::read_all(limit);

    if (parsedOptions.count("no-pager") == 0) {
        auto path = std::experimental::filesystem::temp_directory_path();

        if (!path.empty()) {
            path.append("dlog_log_output");
            std::ofstream stream(path.c_str(), std::ios_base::trunc);
            format::entries(stream, entries.rbegin(), entries.rend());
            stream.close();

            files::open_in_pager(path);
            std::experimental::filesystem::remove(path);

            return 0;
        }
    }

    format::entries(std::cout, entries.rbegin(), entries.rend());

    return 0;
}

std::string subcommands::log::command() const
{
    return "log";
}

std::string subcommands::log::syntax() const
{
    return "[-P] [-l <number>]";
}

std::string subcommands::log::description() const
{
    return "Prints a log of the last n (or all if -l is not specified) entries.\n"
           "  If a pager was found, this command uses paging by default. Otherwise\n"
           "  the output is written to the standard output stream.";
}
