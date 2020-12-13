use chrono::{DateTime, Duration, Local, NaiveTime, TimeZone, Utc};

use crate::errors;

pub fn parse_datetime(string: &str) -> Result<DateTime<Utc>, errors::InvalidFormatError> {
    let trimmed = string.trim();

    DateTime::parse_from_rfc2822(&trimmed)
        .or(DateTime::parse_from_rfc3339(&trimmed))
        .map(|x| { x.with_timezone(&Local) })
        .or(Local.datetime_from_str(&trimmed, "%Y-%m-%d %H:%M:%S"))
        .or(NaiveTime::parse_from_str(&trimmed, "%H:%M:%S").map(|x| { Local::today().and_time(x).expect("invalid format") }))
        .map(|x| { x.with_timezone(&Utc)})
        .map_err(|_| { errors::InvalidFormatError("[%Y-%m-%d] [%H:%M:%S]".into()) })
}

const DURATION_SEGMENTS: &[i64] = &[
    60, // seconds
    60, // minutes
    24, // hours
    7  // days
];

pub fn parse_duration(string: &str) -> Result<Duration, errors::InvalidFormatError> {
    if !string.contains(':') {
        return Err(errors::InvalidFormatError("".into()));
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

        return Err(errors::InvalidFormatError("ww:dd:hh:mm:ss".into()));
    }

    Ok(Duration::seconds(seconds))
}
