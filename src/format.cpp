#include <iomanip>
#include "config.h"
#include "format.h"
#include "cli/color.h"
#include "date.h"

namespace {
    constexpr int TIME_CHAR_COUNT = 2;
    constexpr int DURATION_LENGTH = 12;
    constexpr int ACTIVITY_PROJECT_LENGTH = 36;
}

std::string format::as_local_time_string(const time_t& time)
{
    date time_date(time);
    date now_date;

    char time_string[30];

    strftime(time_string, sizeof(time_string), config::config().time_format.c_str(), localtime(&time));

    std::string result;

    if (now_date.is_yesterday(time_date)) {
        result.append("yesterday at ");
    } else if (now_date != time_date) {
        char date_string[30];

        strftime(date_string, sizeof(date_string), config::config().date_format.c_str(), localtime(&time));

        result.append(date_string);
        result.append(" ");
    }

    result.append(time_string);

    return result;
}

std::ostream& format::local_time(std::ostream& stream, const time_t& time)
{
    tm tm_obj;
    localtime_r(&time, &tm_obj);
    stream << std::put_time(&tm_obj, config::config().time_format.c_str());

    return stream;
}

static std::string pad(const std::string& string, const int& min_chars, const bool& always_pad = false)
{
    if (!always_pad && (string.length() >= min_chars || string == "0")) {
        return string;
    }

    std::string result = std::string(min_chars - string.length(), '0');
    result.append(string);

    return result;
}

std::string format::as_duration(const time_t& duration, const bool& always_pad)
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

    result.append(pad(std::to_string(h.count() % 24), TIME_CHAR_COUNT, always_pad));
    result.append("h ");
    result.append(pad(std::to_string(m.count() % 60), TIME_CHAR_COUNT, always_pad));
    result.append("m ");
    result.append(pad(std::to_string(s.count() % 60), TIME_CHAR_COUNT, always_pad));
    result.append("s");

    return result;
}

std::ostream& format::entry(std::ostream& stream, const entries::entry& entry)
{
    stream << "activity " << cli::color::activity << entry.activity << cli::color::reset;

    if (!entry.project.empty()) {
        stream << " on project " << cli::color::project << entry.project << cli::color::reset;
    }

    if (!entry.tags.empty()) {
        stream << " [";

        for (const std::string& tag : entry.tags) {
            if (tag != *entry.tags.begin()) {
                stream << ", ";
            }

            stream << cli::color::tag << tag << cli::color::reset;
        }

        stream << ']';
    }

    stream << " from " << cli::color::time << format::as_local_time_string(entry.from) << cli::color::reset;
    stream << " to " << cli::color::time << format::as_local_time_string(entry.to) << cli::color::reset;

    stream << " " << cli::color::duration << '(' << format::as_duration(entry.to - entry.from) << ')' << cli::color::reset;

    return stream;
}

static std::ostream& write_entries(std::ostream& stream, std::vector<entries::entry>::const_reverse_iterator begin, const std::vector<entries::entry>::const_reverse_iterator& end)
{
    for (; begin < end; begin++) {
        stream << "    " << cli::color::time;
        format::local_time(stream, begin->from);
        stream << cli::color::reset << " to " << cli::color::time;
        format::local_time(stream, begin->to);
        stream << cli::color::reset << ' ';

        // durations can have varying lengths
        std::string duration = "(";

        duration.append(format::as_duration(begin->to - begin->from, true));
        duration.append(")");
        int went_over_by = duration.length() - DURATION_LENGTH;

        if (went_over_by < 0) {
            went_over_by = 0;
        }

        stream << cli::color::duration << std::setw(DURATION_LENGTH) << duration << cli::color::reset << "  ";

        if (begin->project.empty()) {
            stream << cli::color::activity << std::setw(ACTIVITY_PROJECT_LENGTH - went_over_by) << begin->activity << cli::color::reset;
        } else {
            const int length = ACTIVITY_PROJECT_LENGTH - went_over_by - begin->project.length() - 1;
            stream << cli::color::activity << std::setw(length) << begin->activity << cli::color::reset;
            stream << ':';
            stream << cli::color::project << begin->project << cli::color::reset;
        }

        if (!begin->tags.empty()) {
            stream << "  [";

            for (const auto& tag : begin->tags) {
                stream << cli::color::tag << tag << cli::color::reset;

                if (&tag != &begin->tags.back()) {
                    stream << ", ";
                }
            }

            stream << "]";
        }

        stream << "\n";
    }

    return stream;
}

std::ostream& format::entries(std::ostream& stream, std::vector<entries::entry>::const_iterator begin, const std::vector<entries::entry>::const_iterator& end)
{
    // rang automatically ignores color codes
    // if the output stream is a file, so we
    // have to override this behaviour here
    // to show colours in the pager.
    rang::setControlMode(rang::control::Force);

    // We have to put each iterator entry into a new vector because we want to
    // iterate through the days *in reverse* (starting with the most recent
    // day) but inside of each day we need to show that day's entries starting
    // with the *first* entry of that day, not the last.

    std::vector<entries::entry> date_entries;
    date current_date = date::zero();

    for (; begin != end; begin++) {
        const entries::entry& entry = *begin;
        const date entry_date = date(entry.from);

        if (current_date != entry_date) {
            write_entries(stream, date_entries.rbegin(), date_entries.rend());

            if (!date_entries.empty()) {
                stream << "\n";
            }

            date_entries.clear();

            stream << entry_date << "\n\n";
            current_date = entry_date;
        }

        date_entries.push_back(entry);
    }

    write_entries(stream, date_entries.rbegin(), date_entries.rend());

    rang::setControlMode(rang::control::Auto);

    return stream;
}

bool format::to_int(const std::string& string, int& result)
{
    if (string.empty()) {
        return false;
    }

    std::string input;
    bool is_negative = false;

    if (string.at(0) == '+') {
        input = string.substr(1);
    } else if (string.at(0) == '-') {
        input = string.substr(1);
        is_negative = true;
    } else {
        input = string;
    }

    bool digits_only = (input.find_first_not_of("0123456789") == std::string::npos);

    if (digits_only) {
        result = std::stoi(input);

        if (is_negative) {
            result = -result;
        }

        return true;
    }

    return false;
}
