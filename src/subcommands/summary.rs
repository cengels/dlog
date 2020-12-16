use std::{cmp::Ordering, collections::HashMap, error::Error, vec::IntoIter};
use chrono::{DateTime, Duration, NaiveDateTime, Timelike, Utc};
use clap::Clap;
use colored::Colorize;
use entries::{Entry, EntryCore};
use pager::Pager;
use super::Subcommand;
use crate::{entries, format, parser::parse_datetime};

const DURATION_WIDTH: usize = 30;
const FIELD_WIDTH: usize = 50;

/// Finds the total time spent on certain activities or projects within a given time frame.
/// You can specify multiple filters to further narrow down the result.
/// If no time frame is specified, shows the last thirty days.
#[derive(Clap, Debug)]
pub struct Summary {
    /// Only summarize entries where this string can be found
    /// in its activity, project, tags, or comment.
    #[clap(short, long)]
    string: Option<String>,
    /// Only summarize entries containing this substring in its comment.
    #[clap(short = 'c', long)]
    comment: Option<String>,
    /// Only summarizes entries after this date-time.
    #[clap(short = 'f', long, parse(try_from_str = parse_datetime))]
    from: Option<DateTime<Utc>>,
    /// Only summarizes entries before this date-time.
    #[clap(short = 't', long, parse(try_from_str = parse_datetime))]
    to: Option<DateTime<Utc>>,
    /// Only summarizes entries from today.
    #[clap(short = 'd', long, conflicts_with_all = &["to", "from", "week", "year", "all"])]
    day: bool,
    /// Only summarizes entries from the past seven days.
    #[clap(short = 'w', long, conflicts_with_all = &["to", "from", "day", "year", "all"])]
    week: bool,
    /// Only summarizes entries from the past year.
    #[clap(short = 'y', long, conflicts_with_all = &["to", "from", "day", "week", "all"])]
    year: bool,
    /// Summarizes all entries.
    #[clap(short = 'a', long, conflicts_with_all = &["to", "from", "day", "week", "year"])]
    all: bool,
    /// Limits the summarized entries to the last n entries.
    #[clap(short, long, default_value = "0")]
    limit: u32,
    /// Prints the entries without a pager. Note that this may flood your terminal.
    #[clap(short = 'P', long)]
    no_pager: bool,
    /// An activity and optionally project and tags in the format of
    // <activity>[:<project>] [+<tag>...] to filter the entries.
    activity_project_tags: Vec<String>
}

#[derive(Debug)]
struct Statistics {
    total: Duration,
    activities: HashMap<String, Duration>,
    activities_projects: HashMap<String, Duration>,
    projects: HashMap<String, Duration>,
    tags: HashMap<String, Duration>
}

impl Statistics {
    pub fn new() -> Statistics {
        Statistics {
            total: Duration::seconds(0),
            activities: HashMap::new(),
            activities_projects: HashMap::new(),
            projects: HashMap::new(),
            tags: HashMap::new()
        }
    }
}

impl Subcommand for Summary {
    fn run(&self) -> Result<(), Box<dyn Error>> {
        let entries = entries::read_all()?;

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

fn set(map: &mut HashMap<String, Duration>, key: String, duration: &Duration) {
    if key.is_empty() {
        return;
    }

    if let Some(value) = map.get(&key) {
        if let Some(new_duration) = value.checked_add(duration) {
            map.insert(key, new_duration);
        }
    } else {
        map.insert(key, duration.clone());
    }
}

fn sort(map: &HashMap<String, Duration>) -> IntoIter<(&std::string::String, &chrono::Duration)> {
    let mut vector = map.iter().collect::<Vec<(&String, &Duration)>>();
    vector.sort_by(|a, b| b.1.partial_cmp(&a.1).unwrap_or(Ordering::Equal));

    vector.into_iter()
}

impl Summary {
    fn from(&self) -> DateTime<Utc> {
        self.from.unwrap_or_else(|| {
            if self.day {
                Utc::today().and_hms(0, 0, 0)
            } else if self.week {
                (Utc::today() - Duration::weeks(1)).and_hms(0, 0, 0)
            } else if self.year {
                (Utc::today() - Duration::days(365)).and_hms(0, 0, 0)
            } else if self.all {
                DateTime::<Utc>::from_utc(NaiveDateTime::from_timestamp(0, 0), Utc)
            } else {
                // Default is one month past
                // That's also why it's not one of the CLI flags
                (Utc::today() - Duration::days(30)).and_hms(0, 0, 0)
            }
        })
    }

    fn to(&self) -> DateTime<Utc> {
        self.to.unwrap_or_else(|| Utc::now().with_nanosecond(0).unwrap())
    }

    fn is_filtered(&self, entry: &Entry, entry_core: &EntryCore) -> bool {
        if let Some(string) = &self.string {
            if !entry.activity.contains(string)
             && !entry.project.contains(string)
             && !entry.comment.contains(string)
             && !entry.tags.iter().any(|tag| tag.contains(string)) {
                return true;
            }
        }

        if !entry_core.activity.is_empty() && !entry.activity.contains(&entry_core.activity) {
            return true;
        }

        if !entry_core.project.is_empty() && !entry.project.contains(&entry_core.project) {
            return true;
        }

        for tag in &entry_core.tags {
            if !entry.tags.contains(&tag) {
                return true;
            }
        }

        false
    }

    fn collect_statistics(&self, entries: &Vec<Entry>) -> Statistics {
        let entry_core = self.activity_project_tags.join(" ").parse::<EntryCore>().unwrap();
        let to = self.to();
        let from = self.from();

        let mut stats = Statistics::new();
        let mut counter = 0;

        for entry in entries.iter().rev() {
            if !entry.complete() || !entry.valid() || to < entry.to || self.is_filtered(entry, &entry_core) {
                continue;
            }

            if entry.from < from {
                break;
            }

            let duration = entry.duration();

            if let Some(result) = stats.total.checked_add(&duration) {
                stats.total = result;
            }
            
            set(&mut stats.activities, entry.activity.to_owned(), &duration);

            if !entry.project.is_empty() {
                set(&mut stats.activities_projects, format!("{}:{}", entry.activity, entry.project), &duration);
            }

            set(&mut stats.projects, entry.project.to_owned(), &duration);

            for tag in &entry.tags {
                set(&mut stats.tags, tag.to_owned(), &duration);
            }

            counter += 1;

            if self.limit != 0 && counter >= self.limit {
                break;
            }
        }

        stats
    }

    fn print_results(&self, statistics: &Statistics) {
        if !self.no_pager {
            Pager::new().setup();
        }

        println!("Summary of entries between {} and {}:\n", format::datetime(&self.from()), format::datetime(&self.to()));
        println!("{:w$} {:>dw$}\n", "Total time spent:", format::duration(&statistics.total), w = FIELD_WIDTH, dw = DURATION_WIDTH);
        println!("Activities:\n");

        for activity in sort(&statistics.activities) {
            println!("{:w$} {:>dw$}", activity.0.cyan(), format::duration(&activity.1), w = FIELD_WIDTH, dw = DURATION_WIDTH);
        }

        for activity_project in sort(&statistics.activities_projects) {
            let colon_index = activity_project.0.find(':').unwrap();
            let string = format!("{}:{}", activity_project.0[0..colon_index].cyan(), activity_project.0[colon_index + 1..].bright_red());
            // Due to the ANSI escape code a higher field width is required here.
            println!("{:w$} {:>dw$}", string, format::duration(&activity_project.1), w = FIELD_WIDTH + 18, dw = DURATION_WIDTH);
        }

        if !statistics.projects.is_empty() {
            println!("\nProjects:\n");

            for project in sort(&statistics.projects) {
                println!("{:w$} {:>dw$}", project.0.bright_red(), format::duration(&project.1), w = FIELD_WIDTH, dw = DURATION_WIDTH);
            }
        }

        if !statistics.tags.is_empty() {
            println!("\nTags:\n");

            for tag in sort(&statistics.tags) {
                println!("{:w$} {:>dw$}", tag.0.bright_yellow(), format::duration(&tag.1), w = FIELD_WIDTH, dw = DURATION_WIDTH);
            }
        }
    }
}
