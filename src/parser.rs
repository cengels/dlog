use chrono::{DateTime, Duration, Local, NaiveTime, TimeZone, Utc};

use crate::errors;

/// Attempts to parse a datetime string into a `DateTime<Utc>`.
pub fn parse_datetime(string: &str) -> Result<DateTime<Utc>, errors::InvalidFormatError> {
    let trimmed = string.trim().replace("  ", " ");

    DateTime::parse_from_rfc2822(&trimmed)
        .or_else(|_| DateTime::parse_from_rfc3339(&trimmed))
        .map(|x| { x.with_timezone(&Local) })
        .or_else(|_| Local.datetime_from_str(&trimmed, "%Y-%m-%d %H:%M:%S"))
        .or_else(|_| NaiveTime::parse_from_str(&trimmed, "%H:%M:%S").map_err(|_| ()).and_then(|x| { Local::today().and_time(x).ok_or(()) }))
        .map(|x| { x.with_timezone(&Utc)})
        .or_else(|_| parse_temporal_expression(&trimmed))
        .map_err(|_| { errors::InvalidFormatError("[%Y-%m-%d] %H:%M:%S".to_string()) })
}

fn temporal_expression_error() -> errors::InvalidFormatError {
    return errors::InvalidFormatError("in n seconds/minutes/hours/days OR n seconds/minutes/hours/days ago".to_string());
}

/// Gets the factor necessary to convert the unit into seconds.
fn get_factor(s: &str) -> Option<i64> {
    match s {
        "seconds" | "second" => Some(1),
        "minutes" | "minute" => Some(60),
        "hours" | "hour" => Some(3600),
        "days" | "day" => Some(86400),
        "weeks" | "week" => Some(604800),
        "months" | "month" => Some(2592000),
        "years" | "year" => Some(31536000),
        _ => None
    }
}

/// Parses a temporal expression either in the format of
/// "in n seconds/minutes/hours/days/weeks/months/years" (future) or
/// "n seconds/minutes/hours/days/weeks/months/years ago" (past).
fn parse_temporal_expression(string: &str) -> Result<DateTime<Utc>, errors::InvalidFormatError> {
    // You can also say "now".

    if string == "now" {
        return Ok(Utc::now());
    }

    // We could do this with regexes, but regexes tend to be slow
    // (even though they're actually fairly fast in Rust) so since
    // it's a simple parse, we just do it manually.

    // First we split the string by spaces. A valid temporal expression must always
    // have exactly two spaces, so that's our first check.
    let segments_vec: Vec<&str> = string.split(' ').collect();

    if segments_vec.len() != 3 {
        return Err(temporal_expression_error());
    }

    let segments: (&str, &str, &str) = (segments_vec[0], segments_vec[1], segments_vec[2]);
    let future = segments.0 == "in";

    if !future && segments.2 != "ago" {
        return Err(temporal_expression_error());
    }

    let factor_string = if future { segments.2 } else { segments.1 };
    let factor = get_factor(factor_string).ok_or_else(|| temporal_expression_error())?;
    let number_string = if future { segments.1 } else { segments.0 };
    let number: i64 = number_string.parse().map_err(|_| temporal_expression_error())?;

    if number <= 0 {
        return Err(temporal_expression_error());
    }

    let duration = Duration::seconds(number * factor);

    if future {
        Ok(Utc::now() + duration)
    } else {
        Ok(Utc::now() - duration)
    }
}

const DURATION_SEGMENTS: &[i64] = &[
    60, // seconds
    60, // minutes
    24, // hours
    7  // days
];

/// Attempts to parse a duration string into a `Duration`.
pub fn parse_duration(string: &str) -> Result<Duration, errors::InvalidFormatError> {
    if !string.contains(':') {
        return Err(errors::InvalidFormatError(String::new()));
    }

    let split = string.split(':')
        .rev()  // The first can be anything depending on the number of elements, while the last is always seconds
        .map(|number| { number.parse::<i64>() });
    let mut i = 0;
    let mut seconds: i64 = 0;
    let duration_segment_count = DURATION_SEGMENTS.len();

    for number in split {
        if i <= duration_segment_count {
            if let Ok(number) = number {
                if number >= 0 && (i >= duration_segment_count - 1 || number < DURATION_SEGMENTS[i]) {
                    seconds += if i == 0 { number } else { number * DURATION_SEGMENTS[0..i].iter().product::<i64>() };
                    i += 1;
                    continue;
                }
            }
        }

        return Err(errors::InvalidFormatError("ww:dd:hh:mm:ss".to_string()));
    }

    Ok(Duration::seconds(seconds))
}
