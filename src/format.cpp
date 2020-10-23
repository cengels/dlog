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

static bool is_yesterday(const tm& date, const tm& from_date)
{
    if (from_date.tm_year == date.tm_year) {
        return from_date.tm_yday - 1 == date.tm_yday;
    }

    if (from_date.tm_year - 1 == date.tm_year) {
        return from_date.tm_yday == 0 && date.tm_yday == 365;
    }

    return false;
}

std::string format::as_local_time_string(const time_t& time)
{
    tm date;
    localtime_r(&time, &date);
    const time_t now_t = std::time(nullptr);
    tm now;
    localtime_r(&now_t, &now);

    char time_string[30];

    strftime(time_string, sizeof(time_string), config::config().time_format.c_str(), &date);

    std::string result;

    if (is_yesterday(date, now)) {
        result.append("yesterday at ");
    } else if (date.tm_yday != now.tm_yday && date.tm_year != now.tm_year) {
        char date_string[30];

        strftime(date_string, sizeof(date_string), config::config().date_format.c_str(), &date);

        result.append(date_string);
        result.append("T");
    }

    result.append(time_string);

    return result;
}

std::ostream& format::entry(std::ostream& stream, const entries::entry& entry)
{
    stream << "activity ";
    format::colorize::activity(stream, entry.activity);

    if (!entry.project.empty()) {
        stream << " on project ";
        format::colorize::project(stream, entry.project);
    }

    stream << " ";

    if (!entry.tags.empty()) {
        std::string tag_string = "[";

        for (const std::string& tag : entry.tags) {
            if (tag != *entry.tags.begin()) {
                tag_string.append(", ");
            }

            tag_string.append(tag);
        }

        tag_string.append("]");

        format::colorize::tag(stream, tag_string);
    }

    stream << " from ";
    format::colorize::time(stream, format::as_local_time_string(entry.from));

    stream << " to ";
    format::colorize::time(stream, format::as_local_time_string(entry.to));

    return stream;
}
