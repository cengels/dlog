use std::error::Error;
use chrono::{DateTime, Duration, Timelike, Utc};
use clap::Clap;
use colored::Colorize;
use entries::Entry;
use super::Subcommand;
use crate::{entries::{self, EntryCore}, errors, format, input::parse_datetime, input::parse_duration};

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
    /// Adds a new entry regardless of whether the entry is identical
    /// to the last one.
    #[clap(short = 'n', long)]
    new: bool,
    /// A mandatory activity optionally followed by a project and tags
    /// in the format of <activity>[:<project>] [+<tag>...].
    ///
    /// All components (the activity, project, and tags) can have
    /// spaces in-between them.
    activity_project_tags: Vec<String>
}

impl Subcommand for Fill {
    fn run(&self) -> Result<(), Box<dyn Error>> {
        let mut entries = entries::read_all().unwrap_or_else(|_| Vec::<Entry>::new());
        let last: Entry = entries.last().ok_or(errors::NoEntryError)?.clone();
        let new_entry = self.parse_entry(&last)?;
        let should_update = {
            if self.new { false }
            else if self.update || !last.complete() { true }
            else if new_entry.from != last.from { false }
            else { new_entry.content_equals(&last) }
        };

        if should_update {
            entries.remove(entries.len() - 1);
        }

        entries.push(new_entry.clone());

        entries::rewrite(&entries)?;

        if should_update {
            let duration = new_entry.duration() - last.duration();
            // When negative, we don't add a sign because format::duration()
            // already adds one for us.
            let sign = if duration.num_seconds().is_negative() { "" } else { "+" };
            println!("Updated entry {} {}.", &new_entry, format!("[{}{}]", sign, format::duration(&duration, &format::TimePeriod::Days).clear()).bright_magenta());
        } else {
            println!("Filled entry {}.", &new_entry);
        }

        Ok(())
    }
}

fn get_canonical_to(from: &DateTime<Utc>, to: &Option<DateTime<Utc>>) -> DateTime<Utc> {
    if let Some(to) = to {
        if from.date() < to.date() {
            // Interprets the given `to` as belonging to the previous
            // day if it the time is past midnight
            let previous_day_time = *to - Duration::days(1);

            if *from < previous_day_time {
                return previous_day_time;
            }
        }

        *to
    } else { Utc::now().with_nanosecond(0).unwrap() }
}

impl Fill {
    fn parse_entry(&self, last: &Entry) -> Result<Entry, Box<dyn Error>> {
        if self.duration.is_some() && self.from.is_some() && self.to.is_some() {
            return Err(clap::Error::with_description(String::from("Only two of the three arguments duration, from, and to can be used."), clap::ErrorKind::ArgumentConflict).into());
        }

        let mut entry: Entry = if self.update || !last.complete() { last.clone() } else { Entry::new() };
        let entry_core = self.activity_project_tags.join(" ").parse::<EntryCore>().unwrap();
        
        if !entry_core.activity.is_empty() {
            entry.activity = entry_core.activity;
        }
        if !entry_core.project.is_empty() {
            entry.project = entry_core.project;
        }
        if !entry_core.tags.is_empty() {
            entry.tags = entry_core.tags;
        }

        if entry.activity.is_empty() {
            return Err(clap::Error::with_description(String::from("Please specify at least an activity."), clap::ErrorKind::MissingRequiredArgument).into());
        }

        if let Some(message) = &self.message {
            entry.comment = message.to_owned();
        }

        if let Some(from) = self.from {
            entry.from = from;
        } else if !self.update && last.complete() {
            entry.from = if !self.new && entry.content_equals(last) {
                last.from
            } else {
                last.to
            }
        }

        entry.to = get_canonical_to(&entry.from, &self.to);

        if let Some(duration) = self.duration {
            if self.to.is_some() {
                entry.from = entry.to - duration;
            } else {
                entry.to = entry.from + duration;
            }
        }

        if !entry.valid() {
            println!("{}", entry);
            return Err(errors::InvalidEntryError.into());
        }

        Ok(entry)
    }
}
