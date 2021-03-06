use std::{convert::Infallible, fmt::Display, str::FromStr};
use colored::Colorize;
use serde::{Deserialize, Serialize};
use chrono::{DateTime, Duration, TimeZone, Timelike, Utc, serde::ts_seconds};

use crate::format;

#[derive(Debug)]
pub struct EntryCore {
    pub activity: String,
    pub project: String,
    pub tags: Vec<String>
}

impl FromStr for EntryCore {
    type Err = Infallible;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let mut activity = "";
        let mut project = "";
        let mut s = s;
        let colon_index_opt = s.find(':');

        if let Some(colon_index) = colon_index_opt {
            activity = &s[0..colon_index];
            s = &s[colon_index + 1..];
        }

        if let Some(plus_index) = s.find('+') {
            if colon_index_opt.is_some() {
                project = &s[0..plus_index];
            } else {
                activity = &s[0..plus_index];
            }

            return Ok(Self {
                activity: activity.trim().to_owned(),
                project: project.trim().to_owned(),
                tags: s[plus_index + 1..].split('+').map(|tag| tag.trim().to_owned()).filter(|tag| !tag.is_empty()).collect()
            });
        }
        
        if colon_index_opt.is_some() {
            project = &s[0..];
        } else {
            activity = &s[0..];
        }

        Ok(Self {
            activity: activity.trim().to_owned(),
            project: project.trim().to_owned(),
            tags: Vec::new()
        })
    }
}

#[derive(Serialize, Deserialize, Debug, Clone, PartialEq, Eq)]
pub struct Entry {
    /// A POSIX time point defining the start of the time entry.
    #[serde(with = "ts_seconds")]
    pub from: DateTime<Utc>,
    /// A POSIX time point defining the end of the time entry.
    #[serde(with = "ts_seconds")]
    pub to: DateTime<Utc>,
    /// The activity committed during this time entry.
    /// Must not be an empty string unless the time entry is incomplete.
    pub activity: String,
    /// The project the activity was committed in.
    /// Can be an empty string to mean that the activity does not belong
    /// to a project.
    pub project: String,
    /// A variable number of tags that belong to this time entry.
    #[serde(with = "vector_format")]
    pub tags: Vec<String>,
    /// An optional entry comment.
    pub comment: String
}

impl Entry {
    /// Creates a new incomplete time entry.
    /// Incomplete time entries only have a start point.
    pub fn new() -> Self {
        Self {
            from: Utc::now().with_nanosecond(0).unwrap(),
            to: Utc.timestamp(0, 0),
            activity: String::new(),
            project: String::new(),
            tags: Vec::new(),
            comment: String::new()
        }
    }

    /// Returns `true` if this time entry is valid.
    pub fn valid(&self) -> bool {
        let now = Utc::now();

        self.from.timestamp() > 0  // from must not be uninitialized
         && (self.to.timestamp() == 0 || self.to >= self.from)  // to must be uninitialized or greater than from
         && self.from <= now  // from must be smaller than now
         && self.to <= now  // to must be smaller than now
    }

    /// Returns `true` if this time entry is complete, that is
    /// if it has both a start and an end point.
    pub fn complete(&self) -> bool {
        self.valid()
         && self.to.timestamp() > 0
         && !self.activity.is_empty()
    }

    /// Returns true if the textual contents of the two entries
    /// (i.e. excluding the start and end times) are the same.
    pub fn content_equals(&self, other: &Self) -> bool {
        self.activity == other.activity
         && self.project == other.project
         && self.tags == other.tags
         && self.comment == other.comment
    }

    /// Returns `true` if the time entry is uninitialized.
    pub fn null(&self) -> bool {
        self.from.timestamp() == 0
         && self.to.timestamp() == 0
         && self.activity.is_empty()
         && self.project.is_empty()
         && self.tags.is_empty()
         && self.comment.is_empty()
    }

    /// Gets the duration of this time entry.
    pub fn duration(&self) -> Duration {
        if !self.complete() || !self.valid() {
            return Duration::zero();
        }

        self.to - self.from
    }

    pub fn is_filtered(&self, input: &EntryCore, string: &Option<String>, comment: &Option<String>) -> bool {
        if let Some(string) = string {
            if !self.activity.contains(string)
             && !self.project.contains(string)
             && !self.comment.contains(string)
             && !self.tags.iter().any(|tag| tag.contains(string)) {
                return true;
            }
        }
    
        if !input.activity.is_empty() && self.activity != input.activity {
            return true;
        }
    
        if !input.project.is_empty() && self.project != input.project {
            return true;
        }
    
        if let Some(comment) = comment {
            if !self.comment.contains(comment) {
                return true;
            }
        }
    
        for tag in &input.tags {
            if !self.tags.contains(tag) {
                return true;
            }
        }
    
        false
    }

    /// Gets a colorized string representation of this entry
    /// that can be used as part of a table.
    pub fn tabular(&self) -> String {
        let duration = format::duration_full(&self.duration(), &format::TimePeriod::Hours);

        let activity_project = if self.project.is_empty() {
            self.activity.cyan().to_string()
        } else {
            format!("{}:{}", self.activity.cyan(), self.project.bright_red())
        };

        let tags = if self.tags.is_empty() {
            String::new()
        } else {
            format!("[{}]", self.tags.iter().map(|tag| tag.bright_yellow().to_string()).collect::<Vec<String>>().join(", "))
        };

        let activity_length = if self.project.is_empty() {
            // We have to subtract exactly 9 here for the output
            // to be properly aligned with rows that have empty projects.
            // There is probably a good reason why, but I don't know what it is.
            ACTIVITY_LENGTH - 9
        } else {
            ACTIVITY_LENGTH
        };

        format!("{} to {} {} {:>width$}  {}",
            format::time(&self.from),
            if self.from.date() == self.to.date() { format::time(&self.to) } else { format::datetime_short(&self.to) },
            format!("({})", duration),
            activity_project,
            tags,
            width = activity_length
        )
    }
}

const ACTIVITY_LENGTH: usize = 56;

impl Display for Entry {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let duration = format::duration(&self.duration(), &format::TimePeriod::Days);

        let activity_project = if self.project.is_empty() {
            self.activity.cyan().to_string()
        } else {
            format!("{}:{}", self.activity.cyan(), self.project.bright_red())
        };

        let tags = if self.tags.is_empty() {
            String::new()
        } else {
            format!(" {}", self.tags.iter().map(|tag| (String::from("+") + tag).bright_yellow().to_string()).collect::<Vec<String>>().join(" "))
        };

        f.write_fmt(format_args!("{}{} from {} to {} {}",
            activity_project,
            tags,
            format::time(&self.from),
            if self.from.date() == self.to.date() { format::time(&self.to) } else { format::datetime_short(&self.to) },
            format!("({})", duration)
        ))
    }
}

mod vector_format {
    use serde::{Deserialize, Deserializer, Serializer};

    pub fn serialize<S>(vector: &[String], serializer: S) -> Result<S::Ok, S::Error>
      where S: Serializer
    {
        serializer.serialize_str(&vector.join(","))
    }

    pub fn deserialize<'de, D>(deserializer: D) -> Result<Vec<String>, D::Error>
      where D: Deserializer<'de>,
    {
        let s = String::deserialize(deserializer)?;

        if s.is_empty() {
            Ok(Vec::<String>::new())
        } else {
            Ok(s.split(',').map(ToOwned::to_owned).collect::<Vec<String>>())
        }
    }
}

#[cfg(test)]
mod test {
    use proptest::prelude::*;
    use super::*;
    use crate::test;

    #[test]
    fn test_new() {
        let entry = Entry::new();
        assert_eq!(entry.activity, "");
        assert_eq!(entry.project, "");
        assert_eq!(entry.comment, "");
        assert!(entry.tags.is_empty());
        assert_eq!(entry.from, Utc::now().with_nanosecond(0).unwrap());
        assert_eq!(entry.to, DateTime::parse_from_rfc3339("1970-01-01T00:00:00Z").unwrap());
    }

    proptest! {
        #[test]
        fn test_valid_entry(entry in test::generate_valid_entry()) {
            prop_assert!(entry.valid());
        }
    }

    proptest! {
        #[test]
        fn test_complete_entry(entry in test::generate_complete_entry()) {
            prop_assert!(entry.complete());
        }
    }

    #[test]
    fn test_newly_created_valid() {
        let entry = Entry::new();
        assert!(entry.valid() && !entry.complete());
    }

    #[test]
    fn test_newly_created_with_activity(){
        let mut entry = Entry::new();
        entry.activity = String::from("text");
        assert!(entry.valid() && !entry.complete());
    }

    #[test]
    fn test_unset_from() {
        let mut entry = Entry::new();
        entry.from = DateTime::parse_from_rfc3339("1970-01-01T00:00:00Z").unwrap().with_timezone(&Utc);
        assert!(!entry.valid());
    }

    #[test]
    fn test_from_after_to() {
        let mut entry = Entry::new();
        entry.activity = String::from("text");
        entry.from = DateTime::parse_from_rfc3339("2020-06-19T05:55:00Z").unwrap().with_timezone(&Utc);
        entry.to = DateTime::parse_from_rfc3339("2020-06-18T05:55:00Z").unwrap().with_timezone(&Utc);
        assert!(!entry.valid());
    }

    #[test]
    fn test_future_to() {
        let mut entry = Entry::new();
        entry.activity = String::from("text");
        entry.from = DateTime::parse_from_rfc3339("2020-06-19T05:55:00Z").unwrap().with_timezone(&Utc);
        entry.to = Utc::now() + Duration::weeks(56);
        assert!(!entry.valid());
    }

    fn generate_entry(from: &str, to: &str) -> Entry {
        Entry {
            from: DateTime::parse_from_rfc3339(from).unwrap().with_timezone(&Utc),
            to: DateTime::parse_from_rfc3339(to).unwrap().with_timezone(&Utc),
            activity: String::from("activity"),
            project: String::from("project"),
            tags: vec![String::from("tag1"), String::from("tag2")],
            comment: String::from("This is a comment.")
        }
    }

    #[test]
    fn test_content_equals_true() {
        let entry1 = generate_entry("2020-12-12T08:05:00+00:00", "2020-12-12T16:02:00+00:00");
        let entry2 = generate_entry("2020-12-12T08:05:00+00:00", "2020-12-12T16:30:00+00:00");
        
        assert!(entry1.content_equals(&entry2));
    }

    #[test]
    fn test_content_equals_false() {
        let entry1 = generate_entry("2020-12-12T08:05:00+00:00", "2020-12-12T16:02:00+00:00");
        let mut entry2 = generate_entry("2020-12-12T08:05:00+00:00", "2020-12-12T16:30:00+00:00");
        entry2.comment = String::from("This is also a comment.");
        
        assert!(!entry1.content_equals(&entry2));
    }
}
