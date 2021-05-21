use std::error::Error;
use chrono::{DateTime, Utc};
use clap::Clap;
use colored::Colorize;
use entries::Entry;
use pager::Pager;
use super::Subcommand;
use crate::{entries, format, input::parse_datetime};

/// Prints a log of the last n (or all if -l is not specified) entries.
#[derive(Clap, Debug)]
pub struct Log {
    /// Only prints entries after this date-time.
    #[clap(short = 'f', long, parse(try_from_str = parse_datetime))]
    from: Option<DateTime<Utc>>,
    /// Only prints entries before this date-time.
    #[clap(short = 't', long, parse(try_from_str = parse_datetime))]
    to: Option<DateTime<Utc>>,
    /// Limits the printed entries to the last n entries.
    #[clap(short, long, default_value = "0")]
    limit: u32,
    /// Shows comments as well. Comments are hidden by default to allow for more
    /// compact formatting.
    #[clap(short, long)]
    comments: bool,
    /// Prints the entries without a pager. Note that this may flood your terminal.
    #[clap(short = 'P', long)]
    no_pager: bool
}

impl Subcommand for Log {
    fn run(&self) -> Result<(), Box<dyn Error>> {
        let entries = entries::read_all().unwrap_or_else(|_| Vec::<Entry>::new());

        if entries.is_empty() {
            println!("No entries yet!");
            return Ok(());
        }
        
        if !self.no_pager {
            Pager::new().setup();
        }

        let mut day = Utc::now().date();
        let mut day_entries = Vec::<&Entry>::new();
        let mut counter = 0;

        for entry in entries.iter().rev() {
            if self.from.filter(|from| entry.from < *from).is_some() {
                break;
            }

            if !entry.complete() || !entry.valid() || self.to.filter(|to| *to < entry.from).is_some() {
                continue;
            }

            // We need to print the entries on a day-by-day basis.
            // The days are in reverse order (last to first), but the
            // day's entries are not (first to last).
            // So we need to gather a day's entries first so we can then
            // print them in the reverse (double-reversed) order.
            let entry_day = entry.from.date();

            if entry_day != day {
                self.print_entries_for_day(&day_entries);
                day = entry_day;
                day_entries.clear();
            }

            day_entries.push(entry);
            counter += 1;

            if self.limit != 0 && counter >= self.limit {
                break;
            }
        }

        if !day_entries.is_empty() {
            self.print_entries_for_day(&day_entries);
        }

        Ok(())
    }
}

impl Log {
    fn print_entries_for_day(&self, entries: &[&Entry]) {
        if let Some(first) = entries.first() {
            println!("{}\n", format::date(first.from.date()));

            for entry in entries.iter().rev() {
                println!("    {}", entry.tabular());

                if self.comments && !entry.comment.is_empty() {
                    println!("      {}", entry.comment.bright_cyan());
                }
            }

            println!();
        }
    }
}
