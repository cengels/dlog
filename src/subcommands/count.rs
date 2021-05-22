use std::{collections::HashMap, error::Error};
use chrono::{DateTime, Duration, Local, TimeZone, Timelike, Utc};
use clap::Clap;
use entries::{Entry, EntryCore};
use pager::Pager;
use super::Subcommand;
use crate::{data::{DURATION_WIDTH, FIELD_WIDTH, Statistics}, entries, format, input::parse_datetime};

/// Counts all entries within a given time frame.
/// You can specify multiple filters to further narrow down the result.
/// If no time frame is specified, shows the last thirty days.
#[derive(Clap, Debug)]
pub struct Count {
    /// Only count entries where this string can be found
    /// in its activity, project, tags, or comment.
    #[clap(short, long)]
    string: Option<String>,
    /// Only count entries containing this substring in its comment.
    #[clap(short = 'c', long)]
    comment: Option<String>,
    /// Only count entries after this date-time.
    #[clap(short = 'f', long, parse(try_from_str = parse_datetime))]
    from: Option<DateTime<Utc>>,
    /// Only count entries before this date-time.
    #[clap(short = 't', long, parse(try_from_str = parse_datetime))]
    to: Option<DateTime<Utc>>,
    /// Only count entries from today.
    #[clap(short = 'd', long, conflicts_with_all = &["to", "from", "week", "year", "all"])]
    day: bool,
    /// Only count entries from the past seven days.
    #[clap(short = 'w', long, conflicts_with_all = &["to", "from", "day", "year", "all"])]
    week: bool,
    /// Only count entries from the past year.
    #[clap(short = 'y', long, conflicts_with_all = &["to", "from", "day", "week", "all"])]
    year: bool,
    /// Counts all entries.
    #[clap(short = 'a', long, conflicts_with_all = &["to", "from", "day", "week", "year"])]
    all: bool,
    /// Only counts the last n entries.
    #[clap(short, long, default_value = "0")]
    limit: u32,
    /// Prints the entries without a pager. Note that this may flood your terminal.
    #[clap(short = 'P', long)]
    no_pager: bool,
    /// An activity and optionally project and tags in the format of
    /// `<activity>[:<project>] [+<tag>...]` to filter the entries.
    ///
    /// Activity and project are always fully matched while the tags
    /// allow partial matches.
    activity_project_tags: Vec<String>
}

impl Subcommand for Count {
    fn run(&self) -> Result<(), Box<dyn Error>> {
        let entries = entries::read_all().unwrap_or_else(|_| Vec::<Entry>::new());

        if entries.is_empty() {
            println!("No entries yet!");
            return Ok(());
        }

        if self.to() <= self.from() {
            return Err(clap::Error::with_description(String::from("The start point must be smaller than the end point."), clap::ErrorKind::InvalidValue).into());
        }

        let statistics = self.collect_statistics(&entries);
        self.print_results(&statistics);

        Ok(())
    }
}

fn add_one(map: &mut HashMap<String, u32>, key: String) {
    if key.is_empty() {
        return;
    }

    if let Some(value) = map.get(&key) {
        if let Some(new_count) = value.checked_add(1) {
            map.insert(key, new_count);
        }
    } else {
        map.insert(key, 1);
    }
}

impl Count {
    fn from(&self) -> DateTime<Utc> {
        self.from.unwrap_or_else(|| {
            if self.all {
                return Utc.timestamp(0, 0);
            }

            let result = if self.day {
                Local::today().and_hms(0, 0, 0)
            } else if self.week {
                (Local::today() - Duration::weeks(1)).and_hms(0, 0, 0)
            } else if self.year {
                (Local::today() - Duration::days(365)).and_hms(0, 0, 0)
            } else {
                // Default is one month past
                // That's also why it's not one of the CLI flags
                (Local::today() - Duration::days(30)).and_hms(0, 0, 0)
            };

            result.with_timezone(&Utc)
        })
    }

    fn to(&self) -> DateTime<Utc> {
        self.to.unwrap_or_else(|| Utc::now().with_nanosecond(0).unwrap())
    }

    fn collect_statistics(&self, entries: &[Entry]) -> Statistics<u32> {
        let entry_core = self.activity_project_tags.join(" ").parse::<EntryCore>().unwrap();
        let to = self.to();
        let from = self.from();

        let mut stats = Statistics::<u32>::new();

        for entry in entries.iter().rev() {
            if entry.from < from {
                break;
            }

            if !entry.complete() || !entry.valid() || to < entry.from || entry.is_filtered(&entry_core, &self.string, &self.comment) {
                continue;
            }

            stats.total += 1;
            
            add_one(&mut stats.activities, entry.activity.to_owned());

            if !entry.project.is_empty() {
                add_one(&mut stats.activities_projects, format!("{}:{}", entry.activity, entry.project));
            }

            add_one(&mut stats.projects, entry.project.to_owned());

            for tag in &entry.tags {
                add_one(&mut stats.tags, tag.to_owned());
            }

            if self.limit != 0 && stats.total >= self.limit {
                break;
            }
        }

        stats
    }

    fn print_results(&self, statistics: &Statistics<u32>) {
        if !self.no_pager {
            Pager::new().setup();
        }

        println!("Count of entries between {} and {}:\n", format::datetime(&self.from()), format::datetime(&self.to()));
        println!("{:w$} {:>dw$}\n", "Total number of entries:", format::count(&statistics.total), w = FIELD_WIDTH, dw = DURATION_WIDTH);

        statistics.print(|count| format::count(count));
    }
}
