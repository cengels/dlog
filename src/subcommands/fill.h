#include "cxxopts.hpp"
#include "subcommand.h"

#pragma once

namespace subcommands {
    class fill : public subcommands::subcommand
    {
        public:
            int run(const cxxopts::ParseResult& parsedOptions) override;
            std::string command() const override;
            std::string syntax() const override;
            std::string description() const override;
            virtual cxxopts::Options options() const override;
    };
}
