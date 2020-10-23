#include "config.h"
#include "rang.hpp"
#include "format.h"

std::ostream& format::colorize::error(std::ostream& stream, const std::string& string)
{
    return stream << rang::fg::red << string << rang::fg::reset;
}

std::ostream& format::colorize::project(std::ostream& stream, const std::string& string)
{
    return stream << rang::fgB::red << string << rang::fg::reset;
}

std::ostream& format::colorize::activity(std::ostream& stream, const std::string& string)
{
    return stream << rang::fg::cyan << string << rang::fg::reset;
}

std::ostream& format::colorize::time(std::ostream& stream, const std::string& string)
{
    return stream << rang::fg::magenta << string << rang::fg::reset;
}

std::ostream& format::colorize::tag(std::ostream& stream, const std::string& string)
{
    return stream << rang::fgB::yellow << string << rang::fg::reset;
}

std::ostream& format::colorize::command(std::ostream& stream, const std::string& string)
{
    return stream << rang::fg::blue << string << rang::fg::reset;
}

std::string format::as_local_time_string(const time_t& time)
{
    char time_string[30];

    strftime(time_string, sizeof(time_string), config::config().time_format.c_str(), localtime(&time));

    return std::string(time_string);
}
