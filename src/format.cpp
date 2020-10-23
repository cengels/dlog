#include "config.h"
#include "rang.hpp"
#include "format.h"

namespace {
    constexpr int TIME_CHAR_COUNT = 2;
}

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

std::ostream& format::colorize::duration(std::ostream& stream, const std::string& string)
{
    return stream << rang::fgB::magenta << string << rang::fg::reset;
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

    if (now_t - 10 <= time) {
        return "now";
    }

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

static std::string pad(const std::string& string, int min_chars)
{
    if (string.length() >= min_chars) {
        return string;
    }

    std::string result = std::string(min_chars - string.length(), '0');
    result.append(string);

    return result;
}

std::string format::as_duration(const time_t& duration)
{
    std::string result;
    using days = std::chrono::duration<int, std::ratio<86400>>;
    using hours = std::chrono::hours;
    using minutes = std::chrono::minutes;
    using seconds = std::chrono::seconds;

    const seconds s = seconds(duration);
    const minutes m = std::chrono::duration_cast<minutes>(s);
    const hours h = std::chrono::duration_cast<hours>(m);
    const days d = std::chrono::duration_cast<days>(h);

    if (d.count() >= 1) {
        result.append(std::to_string(d.count()));
        result.append("d ");
    }

    result.append(pad(std::to_string(h.count() % 24), TIME_CHAR_COUNT));
    result.append("h ");
    result.append(pad(std::to_string(m.count() % 60), TIME_CHAR_COUNT));
    result.append("m ");
    result.append(pad(std::to_string(s.count() % 60), TIME_CHAR_COUNT));
    result.append("s");

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

    if (!entry.tags.empty()) {
        stream << " ";

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

    stream << " ";
    std::string duration = "(";
    duration.append(format::as_duration(entry.to - entry.from));
    duration.append(")");
    format::colorize::duration(stream, duration);

    return stream;
}
