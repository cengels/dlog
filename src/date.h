#include <time.h>
#include <iostream>

#pragma once

/** A date object encompassing the day, month, and year. */
class date {
    public:
        /** Constructs a new date pointing to the current day. */
        date();
        /** Constructs a new date from the given POSIX time point. */
        date(const time_t& time);

        /** Gets this date's day of the month (1-31). */
        int day() const;
        /** Gets this date's month (1-12). */
        int month() const;
        /** Gets this date's year. */
        int year() const;

        /** Returns true if this date corresponds to the current local system date. */
        bool today() const;
        /** Returns true if this date corresponds to yesterday, according to the current local system date. */
        bool yesterday() const;

        /** Checks if the given date is the day before this one. */
        bool is_yesterday(const date& yesterday) const;
        /** Checks if the given time evaluates to the day before this one. */
        bool is_yesterday(const time_t& yesterday) const;

        /**
         * Returns true if the year represented by this date is a leap year.
         * In that case there are 366 days in the year, not 365.
         */
        bool is_leap_year() const;

        /** Gets the number of days in the month represented by this date. */
        int days_in_month() const;

        bool operator<(const date& other) const;
        bool operator>(const date& other) const;
        bool operator<=(const date& other) const;
        bool operator>=(const date& other) const;
        bool operator==(const date& other) const;
        bool operator!=(const date& other) const;

        friend std::ostream& operator<<(std::ostream& stream, const date& date);

        /** Returns a new date that corresponds to January 1, 1970. */
        static date zero();

    private:
        date(const time_t& time, const tm* tm_obj);

        time_t m_time;
        int m_day;
        int m_month;
        int m_year;
};
