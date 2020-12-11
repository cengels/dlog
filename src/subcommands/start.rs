use std::error::Error;
use clap::Clap;
use entries::Entry;
use super::Subcommand;
use crate::{entries, errors};

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

        entries.push(Entry::new());

        entries::rewrite(&entries)?;

        Ok(())
    }
}
