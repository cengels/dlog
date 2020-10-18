#include "cxxopts.hpp"
#include "subcommand.h"

#pragma once

namespace subcommands {
    class main : public subcommands::subcommand
    {
        public:
            int run(const cxxopts::ParseResult& parsedOptions) override;
            std::string command() const override;
            std::string description() const override;
            std::string help() const override;
            virtual cxxopts::Options options() const override;
    };
}
