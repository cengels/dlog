use std::{error::Error, io::Write};
use clap::Clap;
use entries::Entry;
use super::Subcommand;
use crate::{entries, errors, format};

/// Removes a time entry from the list.
///
/// EXAMPLES:
///     dlog remove       removes the last time entry
///     dlog remove 2     removes the second-to-last time entry
#[derive(Clap, Debug)]
#[clap(verbatim_doc_comment)]
pub struct Remove {
    /// Suppresses the confirmation dialog before removal.
    #[clap(short = 'y', long)]
    yes: bool,
    /// A number where the number of entries - which specifies
    /// the entry to be removed. Note that this number is
    /// one-indexed.
    #[clap(default_value = "1")]
    which: usize
}

impl Subcommand for Remove {
    fn run(&self) -> Result<(), Box<dyn Error>> {
        let mut entries = entries::read_all().unwrap_or_else(|_| Vec::<Entry>::new());

        if entries.is_empty() {
            return Err(errors::NoEntryError.into());
        }

        let len = entries.len();
        let index = entries.len() - self.which;
        let opt_entry = entries.get(index);

        if self.which >= len || opt_entry.is_none() {
            return Err(clap::Error::with_description("No entry with the specified index found.".into(), clap::ErrorKind::InvalidValue).into());
        }

        let entry = opt_entry.unwrap();
        let entry_string = if entry.complete() {
            format!("entry {}", entry)
        } else {
            format!("incomplete entry started {}", format::datetime(&entry.from))
        };

        if !self.yes {
            print!("Do you want to remove {}? (y/n) ", entry_string);
            std::io::stdout().flush()?;

            let mut input = String::new();
            std::io::stdin().read_line(&mut input)?;
            let input = input.trim().to_lowercase();
    
            if input != "y" && input != "yes" {
                println!("No entry removed.");
                return Ok(());
            }
        }

        entries.remove(index);

        entries::rewrite(&entries)?;

        println!("Removed {}", entry_string);

        Ok(())
    }
}
