use chrono::{Date, DateTime, Duration, Local, Utc};
use colored::{ColoredString, Colorize};

const FORMAT_DATE_TODAY: &str = "%A, %d %B %Y (today)";
const FORMAT_DATE_YESTERDAY: &str = "%A, %d %B %Y (yesterday)";
const FORMAT_DATE: &str = "%A, %d %B %Y";
const FORMAT_DATETIME_TODAY: &str = "today at %H:%M:%S";
const FORMAT_DATETIME_YESTERDAY: &str = "yesterday at %H:%M:%S";
const FORMAT_DATETIME: &str = "%A, %d %B %Y at %H:%M:%S";
const FORMAT_DATETIME_SHORT: &str = "%Y-%m-%d %H:%M:%S";
const FORMAT_TIME: &str = "%H:%M:%S";

pub fn time(date: &DateTime<Utc>) -> ColoredString {
    date.with_timezone(&Local).format(FORMAT_TIME).to_string().magenta()
}

pub fn datetime_short(date: &DateTime<Utc>) -> ColoredString {
    date.with_timezone(&Local).format(FORMAT_DATETIME_SHORT).to_string().magenta()
}

pub fn date(date: Date<Utc>) -> ColoredString {
    let today = Local::today();
    let date = date.with_timezone(&Local);

    let delayed_format = if date == today {
        date.format(FORMAT_DATE_TODAY)
    } else if date == (today - Duration::days(1)) {
        date.format(FORMAT_DATE_YESTERDAY)
    } else {
        date.format(FORMAT_DATE)
    };

    delayed_format.to_string().replace(" 0", " ").green()
}

pub fn datetime(datetime: &DateTime<Utc>) -> ColoredString {
    if datetime.timestamp() == 0 {
        return "the beginning of time".magenta();
    }

    let today = Utc::today();
    let datetime = datetime.with_timezone(&Local);

    let delayed_format = if datetime.date() == today {
        datetime.format(FORMAT_DATETIME_TODAY)
    } else if datetime.date() == (today - Duration::days(1)) {
        datetime.format(FORMAT_DATETIME_YESTERDAY)
    } else {
        datetime.format(FORMAT_DATETIME)
    };

    delayed_format.to_string().magenta()
}

#[derive(PartialEq, PartialOrd)]
pub enum TimePeriod {
    Days = 3,
    Hours = 2,
    Minutes = 1,
    Seconds = 0
}

pub fn duration(duration: &Duration, largest_period: &TimePeriod) -> ColoredString {
    let days = duration.num_days().abs();
    let hours = if largest_period == &TimePeriod::Hours { duration.num_hours().abs() } else { duration.num_hours().abs() % 24 };
    let minutes = if largest_period == &TimePeriod::Minutes { duration.num_minutes().abs() } else { duration.num_minutes().abs() % 60 };
    let seconds = if largest_period == &TimePeriod::Seconds { duration.num_seconds().abs() } else { duration.num_seconds().abs() % 60 };
    let sign = if duration.num_seconds().is_negative() { "-" } else { "" };

    let string = if days != 0 && largest_period >= &TimePeriod::Days {
        format!("{}{:0>2}d {:0>2}h {:0>2}m {:0>2}s", sign, days, hours, minutes, seconds)
    } else if hours != 0 && largest_period >= &TimePeriod::Hours {
        format!("{}{:0>2}h {:0>2}m {:0>2}s", sign, hours, minutes, seconds)
    } else if minutes != 0 && largest_period >= &TimePeriod::Minutes {
        format!("{}{:0>2}m {:0>2}s", sign, minutes, seconds)
    } else {
        format!("{}{:0>2}s", sign, seconds)
    };

    string.bright_magenta()
}
