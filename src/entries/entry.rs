use serde::{Serialize, Deserialize};
use chrono::{Utc, DateTime};

#[derive(Serialize, Deserialize)]
pub struct Entry<'a> {
    /// A POSIX time point defining the start of the time entry.
    from: DateTime<Utc>,
    /// A POSIX time point defining the end of the time entry.
    to: DateTime<Utc>,
    /// The activity committed during this time entry.
    /// Must not be an empty string unless the time entry is incomplete.
    activity: &'a str,
    /// The project the activity was committed in.
    /// Can be an empty string to mean that the activity does not belong
    /// to a project.
    project: &'a str,
    /// A variable number of tags that belong to this time entry.
    tags: Vec<&'a str>,
    /// An optional entry comment.
    comment: &'a str
}

impl Entry<'_> {
    /// Returns `true` if this time entry is valid.
    pub fn valid(&self) -> bool {
        let now = Utc::now();

        self.from.timestamp_millis() > 0  // from must not be uninitialized
         && (self.to.timestamp_millis() == 0 || self.to >= self.from)  // to must be uninitialized or greater than from
         && self.from <= now  // from must be smaller than now
         && self.to <= now  // to must be smaller than now
    }

    /// Returns `true` if this time entry is complete, that is
    /// if it has both a start and an end point.
    pub fn complete(&self) -> bool {
        self.valid()
         && self.to.timestamp_millis() > 0
         && !self.activity.is_empty()
    }

    /// Returns `true` if the time entry is uninitialized.
    pub fn null(&self) -> bool {
        self.from.timestamp_millis() == 0
         && self.to.timestamp_millis() == 0
         && self.activity.is_empty()
         && self.project.is_empty()
         && self.tags.is_empty()
         && self.comment.is_empty()
    }
}
