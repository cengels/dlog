#include <string>
#include "cxxopts.hpp"

#pragma once

namespace subcommands {
    /**
     * A subcommand invokable from the command line.
     *
     * To use, inherit from subcommand, define a command and descrpition,
     * and override subcommand::run() to define what should happen
     * if the user calls this subcommand.
     */
    class subcommand {
        public:
            virtual ~subcommand() = default;

            /** Runs the subcommand with the given parsed options. */
            virtual int run(const cxxopts::ParseResult& parsedOptions);

            /** Gets the command that must be called from the command line to invoke this subcommand. */
            virtual std::string command() const = 0;

            /** Gets a string describing this command. */
            virtual std::string description() const = 0;

            /** Gets a string containing information about how this subcommand is to be used. */
            virtual std::string help() const;

            /** Gets the available command line options and usage information for this subcommand. */
            virtual cxxopts::Options options() const;
    };
}
