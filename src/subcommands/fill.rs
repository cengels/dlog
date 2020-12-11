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
    positionals: String
}

impl Subcommand for Fill {
    fn run(&self) -> Result<(), Box<dyn Error>> {
        let mut entries = entries::read_all()?;
        let last = entries.last().ok_or(errors::NoEntryError)?;
        let new_entry = self.parse_entry(&last)?;

        if self.duration.is_some() && self.from.is_some() && self.to.is_some() {
            return Err(clap::Error::with_description("Only two of the three arguments duration, from, and to can be used.".into(), clap::ErrorKind::ArgumentConflict).into());
        }

        println!("self: {:?}", self);
        println!("entry: {:?}", new_entry);

        // entries.push(new_entry);

        // entries::rewrite(&entries)?;

        Ok(())
    }
}

impl Fill {
    fn parse_entry(&self, last: &Entry) -> Result<Entry, Box<dyn Error>> {
        Ok(Entry::new())
    }
}
