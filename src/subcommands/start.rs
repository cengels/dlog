use std::error::Error;
use clap::Clap;
use entries::Entry;
use super::Subcommand;
use crate::{entries, errors, format};

/// Starts a new incomplete time entry. Call dlog fill to stop it.
#[derive(Clap)]
pub struct Start;

impl Subcommand for Start {
    fn run(&self) -> Result<(), Box<dyn Error>> {
        let mut entries = entries::read_all()?;

        if let Some(last) = entries.last() {
            if !last.complete() {
                return Err(Box::new(errors::IncompleteEntryError))
            }
        }

        let new_entry = Entry::new();
        let from = format::datetime(&new_entry.from);

        entries.push(new_entry);

        entries::rewrite(&entries)?;

        if from.contains("at") {
            println!("Started new incomplete time entry {}.", &from);
        } else {
            println!("Started new incomplete time entry at {}.", &from);
        }

        Ok(())
    }
}
