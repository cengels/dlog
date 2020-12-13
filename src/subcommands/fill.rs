use std::error::Error;
use chrono::{DateTime, Duration, Utc};
use clap::Clap;
use entries::Entry;
use super::Subcommand;
use crate::{entries, errors, parser::parse_datetime, parser::parse_duration};

/// Fills the time between the last time entry and now with a new time entry.
#[derive(Clap, Debug)]
pub struct Fill {
    /// A date-time, time, or temporal expression when the entry started.
    #[clap(short = 'f', long, parse(try_from_str = parse_datetime))]
    from: Option<DateTime<Utc>>,
    /// A date-time, time, or temporal expression when the entry stopped.
    #[clap(short = 't', long, parse(try_from_str = parse_datetime))]
    to: Option<DateTime<Utc>>,
    /// A duration to say how long the entry lasted from start.
    #[clap(short = 'd', long, parse(try_from_str = parse_duration))]
    duration: Option<Duration>,
    /// A comment to be added to the entry.
    #[clap(short = 'm', long)]
    message: Option<String>,
    /// Updates the last entry instead of adding a new one.
    #[clap(short = 'u', long)]
    update: bool,
    /// A mandatory activity optionally followed by a project and tags
    /// in the format of "ACTIVITY:PROJECT +TAG1 +TAG2".
    ///
    /// All components (the activity, project, and tags) can have
    /// spaces in-between them.
    positionals: Vec<String>
}

impl Subcommand for Fill {
    fn run(&self) -> Result<(), Box<dyn Error>> {
        let mut entries = entries::read_all()?;
        let last = entries.last().ok_or(errors::NoEntryError)?;
        let new_entry = self.parse_entry(&last)?;

        if self.update {
            entries.remove(entries.len() - 1);
        }

        entries.push(new_entry);

        entries::rewrite(&entries)?;

        Ok(())
    }
}

impl Fill {
    fn parse_entry(&self, last: &Entry) -> Result<Entry, Box<dyn Error>> {
        if self.duration.is_some() && self.from.is_some() && self.to.is_some() {
            return Err(clap::Error::with_description("Only two of the three arguments duration, from, and to can be used.".into(), clap::ErrorKind::ArgumentConflict).into());
        }

        let mut entry: Entry = if self.update { last.clone() } else { Entry::new() };

        let mut activity_project = String::new();
        let mut tag_mode = false;

        for positional in &self.positionals {
            if !tag_mode && positional.starts_with('+') {
                tag_mode = true;
            }

            if tag_mode {
                if positional.starts_with('+') {
                    entry.tags.push(positional[1..].into());
                } else if let Some(last) = entry.tags.last_mut() {
                    last.push(' ');
                    last.push_str(positional);
                }
            } else {
                activity_project.push(' ');
                activity_project += positional;
            }
        }

        activity_project = activity_project.trim().into();

        if !activity_project.is_empty() {
            let mut activity_mode = true;

            for string in activity_project.splitn(2, ':') {
                if activity_mode {
                    entry.activity = string.to_string();
                    activity_mode = false;
                } else {
                    entry.project = string.to_string();
                }
            }
        }

        if entry.activity.is_empty() {
            return Err(clap::Error::with_description("Please specify at least an activity.".into(), clap::ErrorKind::MissingRequiredArgument).into());
        }

        if let Some(to) = self.to {
            entry.to = to;
        } else {
            entry.to = Utc::now();
        }

        if let Some(from) = self.from {
            entry.from = from;
        } else if !self.update {
            entry.from = last.to.clone();
        }

        if let Some(duration) = self.duration {
            if self.to.is_some() {
                entry.from = entry.to - duration;
            } else {
                entry.to = entry.from + duration;
            }
        }

        if let Some(message) = &self.message {
            entry.comment = message.to_string();
        }

        Ok(entry)
    }
}
