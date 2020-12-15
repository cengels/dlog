use chrono::{Date, DateTime, Duration, Utc};
use colored::{ColoredString, Colorize};

const FORMAT_DATE_TODAY: &str = "%A, %d %B %Y (today)";
const FORMAT_DATE_YESTERDAY: &str = "%A, %d %B %Y (yesterday)";
const FORMAT_DATE: &str = "%A, %d %B %Y";
const FORMAT_DATETIME_TODAY: &str = "today at %H:%M:%S";
const FORMAT_DATETIME_YESTERDAY: &str = "yesterday at %H:%M:%S";
const FORMAT_DATETIME: &str = "%A, %d %B %Y at %H:%M:%S";

pub fn date(date: &Date<Utc>) -> ColoredString {
    let today = Utc::now().date();

    let delayed_format = if *date == today {
        date.format(FORMAT_DATE_TODAY)
    } else if *date == (today - Duration::days(1)) {
        date.format(FORMAT_DATE_YESTERDAY)
    } else {
        date.format(FORMAT_DATE)
    };

    delayed_format.to_string().replace(" 0", " ").green()
}

pub fn datetime(datetime: &DateTime<Utc>) -> ColoredString {
    let today = Utc::now().date();

    let delayed_format = if datetime.date() == today {
        datetime.format(FORMAT_DATETIME_TODAY)
    } else if datetime.date() == (today - Duration::days(1)) {
        datetime.format(FORMAT_DATETIME_YESTERDAY)
    } else {
        datetime.format(FORMAT_DATETIME)
    };

    delayed_format.to_string().magenta()
}

pub fn duration(duration: &Duration) -> ColoredString {
    let days = duration.num_days();

    let string = if days > 0 {
        format!("{:0>2}d {:0>2}h {:0>2}m {:0>2}s", days, duration.num_hours() % 24, duration.num_minutes() % 60, duration.num_seconds() % 60)
    } else {
        format!("{:0>2}h {:0>2}m {:0>2}s", duration.num_hours() % 24, duration.num_minutes() % 60, duration.num_seconds() % 60)
    };

    string.bright_magenta()
}
