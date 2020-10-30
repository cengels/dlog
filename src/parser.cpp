#include <iomanip>
#include <algorithm>
#include "config.h"
#include "format.h"
#include "parser.h"

time_t parser::parse_relative_time(const std::string& string)
{
    int temporal_count = 0;
    char temporal_type_char[20];

    if (sscanf(string.c_str(), "%d %s", &temporal_count, temporal_type_char) != EOF) {
        if (temporal_count <= 0) {
            return 0;
        }

        const bool is_plural = temporal_count != 1;
        const std::string temporal_type = std::string(temporal_type_char);

        if (is_plural ? temporal_type == "seconds" : temporal_type == "second") {
            return temporal_count;
        } else if (is_plural ? temporal_type == "minutes" : temporal_type == "minute") {
            return temporal_count * 60;
        } else if (is_plural ? temporal_type == "hours" : temporal_type == "hour") {
            return temporal_count * 3600;
        } else if (is_plural ? temporal_type == "days" : temporal_type == "day") {
            return temporal_count * 86400;
        } else if (is_plural ? temporal_type == "weeks" : temporal_type == "week") {
            return temporal_count * 604800;
        }

        return 0;
    }
}

time_t parser::parse_temporal(const std::string& string)
{
    int index_of_ago = string.find(" ago");

    if (index_of_ago != std::string::npos) {
        time_t relative_time = parser::parse_relative_time(string.substr(0, index_of_ago));

        if (relative_time > 0) {
            time_t now = std::time(nullptr);

            return now - relative_time;
        }

        return 0;
    }

    std::string date_time(string);

    if (date_time.find(' ') == std::string::npos) {
        // No date, so we prepend the current date manually to allow
        // get_time() to use the correct date.

        std::string date_string = format::date_string(std::time(nullptr));
        date_string.push_back(' ');
        date_time.insert(0, date_string);
    }

    struct std::tm tm;
    std::istringstream stream(date_time);
    std::string format = config::config().date_format + " " + config::config().time_format;

    stream >> std::get_time(&tm, format.c_str());

    if (stream.fail()) {
        return 0;
    }

    // This line is necessary to ensure that mktime() leaves
    // the time as-is instead of subtracting one hour due to
    // daylight savings time. If we didn't do this, there
    // would be off-by-one hour errors in the returned time.
    tm.tm_isdst = -1;

    return mktime(&tm);
}
