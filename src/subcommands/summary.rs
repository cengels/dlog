use std::{cmp::Ordering, collections::HashMap, error::Error, vec::IntoIter};
use chrono::{DateTime, Duration, Local, TimeZone, Timelike, Utc};
use clap::Clap;
use colored::Colorize;
use entries::{Entry, EntryCore};
use format::TimePeriod;
use pager::Pager;
use super::Subcommand;
use crate::{entries, format, input::parse_datetime};

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
    /// Always displays totaled durations in hours, not days.
    #[clap(short = 'h', long)]
    hours: bool,
    /// An activity and optionally project and tags in the format of
    /// `<activity>[:<project>] [+<tag>...]` to filter the entries.
    ///
    /// Activity and project are always fully matched while the tags
    /// allow partial matches.
    activity_project_tags: Vec<String>
}

#[derive(Debug, PartialEq)]
struct Statistics {
    total: Duration,
    activities: HashMap<String, Duration>,
    activities_projects: HashMap<String, Duration>,
    projects: HashMap<String, Duration>,
    tags: HashMap<String, Duration>
}

impl Statistics {
    pub fn new() -> Self {
        Self {
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

fn set(map: &mut HashMap<String, Duration>, key: String, duration: &Duration) {
    if key.is_empty() {
        return;
    }

    if let Some(value) = map.get(&key) {
        if let Some(new_duration) = value.checked_add(duration) {
            map.insert(key, new_duration);
        }
    } else {
        map.insert(key, *duration);
    }
}

fn sort(map: &HashMap<String, Duration>) -> IntoIter<(&std::string::String, &chrono::Duration)> {
    let mut vector = map.iter().collect::<Vec<(&String, &Duration)>>();
    vector.sort_by(|a, b| b.1.partial_cmp(a.1).unwrap_or(Ordering::Equal));

    vector.into_iter()
}

impl Summary {
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

    fn is_filtered(&self, entry: &Entry, entry_core: &EntryCore) -> bool {
        if let Some(string) = &self.string {
            if !entry.activity.contains(string)
             && !entry.project.contains(string)
             && !entry.comment.contains(string)
             && !entry.tags.iter().any(|tag| tag.contains(string)) {
                return true;
            }
        }

        if !entry_core.activity.is_empty() && entry.activity != entry_core.activity {
            return true;
        }

        if !entry_core.project.is_empty() && entry.project != entry_core.project {
            return true;
        }

        if let Some(comment) = &self.comment {
            if !entry.comment.contains(comment) {
                return true;
            }
        }

        for tag in &entry_core.tags {
            if !entry.tags.contains(tag) {
                return true;
            }
        }

        false
    }

    fn collect_statistics(&self, entries: &[Entry]) -> Statistics {
        let entry_core = self.activity_project_tags.join(" ").parse::<EntryCore>().unwrap();
        let to = self.to();
        let from = self.from();

        let mut stats = Statistics::new();
        let mut counter = 0;

        for entry in entries.iter().rev() {
            if entry.from < from {
                break;
            }

            if !entry.complete() || !entry.valid() || to < entry.from || self.is_filtered(entry, &entry_core) {
                continue;
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

        let time_period = if self.hours { TimePeriod::Hours } else { TimePeriod::Days };

        println!("Summary of entries between {} and {}:\n", format::datetime(&self.from()), format::datetime(&self.to()));
        println!("{:w$} {:>dw$}\n", "Total time spent:", format::duration(&statistics.total, &time_period), w = FIELD_WIDTH, dw = DURATION_WIDTH);
        println!("Activities:\n");

        for activity in sort(&statistics.activities) {
            println!("{:w$} {:>dw$}", activity.0.cyan(), format::duration(activity.1, &time_period), w = FIELD_WIDTH, dw = DURATION_WIDTH);
        }

        for activity_project in sort(&statistics.activities_projects) {
            let colon_index = activity_project.0.find(':').unwrap();
            let string = format!("{}:{}", activity_project.0[0..colon_index].cyan(), activity_project.0[colon_index + 1..].bright_red());
            // Due to the ANSI escape code a higher field width is required here.
            println!("{:w$} {:>dw$}", string, format::duration(activity_project.1, &time_period), w = FIELD_WIDTH + 18, dw = DURATION_WIDTH);
        }

        if !statistics.projects.is_empty() {
            println!("\nProjects:\n");

            for project in sort(&statistics.projects) {
                println!("{:w$} {:>dw$}", project.0.bright_red(), format::duration(project.1, &time_period), w = FIELD_WIDTH, dw = DURATION_WIDTH);
            }
        }

        if !statistics.tags.is_empty() {
            println!("\nTags:\n");

            for tag in sort(&statistics.tags) {
                println!("{:w$} {:>dw$}", tag.0.bright_yellow(), format::duration(tag.1, &time_period), w = FIELD_WIDTH, dw = DURATION_WIDTH);
            }
        }
    }
}


#[cfg(test)]
mod test {
    use std::collections::HashMap;
    use chrono::{Duration, TimeZone, Utc};
    use crate::test;
    use super::{Statistics, Summary};

    fn new_summary() -> Summary {
        Summary {
            // From May 1, 2012 to May 12, 2012
            from: Utc.timestamp_opt(1335826800, 0).earliest(),
            to: Utc.timestamp_opt(1336777199, 0).earliest(),
            all: false,
            week: false,
            year: false,
            day: false,
            string: None,
            comment: None,
            limit: 0,
            no_pager: false,
            hours: false,
            activity_project_tags: Vec::new()
        }
    }

    fn stats_for(timestamp: i64, days: i64) -> Statistics {
        let mut summary = new_summary();
        summary.to = Some(Utc.timestamp(timestamp, 0));
        summary.from = Some(summary.to.unwrap() - Duration::days(days));
        summary.collect_statistics(&test::read_test_entries())
    }

    fn map(arr: &[(&str, i64)]) -> HashMap<String, Duration> {
        arr.iter().map(|(name, number)| (String::from(*name), Duration::seconds(*number))).collect()
    }

    #[test]
    fn test_empty_period() {
        let stats = stats_for(1322376000, 4);

        assert!(stats.activities.is_empty()
            && stats.activities_projects.is_empty()
            && stats.projects.is_empty()
            && stats.tags.is_empty()
            && stats.total.is_zero());
    }

    #[test]
    fn test_default_args() {
        let expected = Statistics {
            total: Duration::seconds(416204),
            activities: map(&[("orci vehicula", 373660), ("erat tortor", 42544)]),
            activities_projects: map(&[("orci vehicula:cursus urna", 373660), ("erat tortor:ac", 42544)]),
            projects: map(&[("cursus urna", 373660), ("ac", 42544)]),
            tags: map(&[("in", 373660), ("habitasse", 373660), ("hac", 373660),
                        ("metus", 42544), ("ut", 42544), ("sapien", 42544)])
        };

        assert_eq!(expected, new_summary().collect_statistics(&test::read_test_entries()));
    }

    #[test]
    fn test_string() {
        let expected = Statistics {
            total: Duration::seconds(373660),
            activities: map(&[("orci vehicula", 373660)]),
            activities_projects: map(&[("orci vehicula:cursus urna", 373660)]),
            projects: map(&[("cursus urna", 373660)]),
            tags: map(&[("in", 373660), ("habitasse", 373660), ("hac", 373660)])
        };

        let mut summary = new_summary();
        summary.string = Some(String::from("ass"));

        assert_eq!(expected, summary.collect_statistics(&test::read_test_entries()));
    }

    #[test]
    fn test_comment() {
        let expected = Statistics {
            total: Duration::seconds(42544),
            activities: map(&[("erat tortor", 42544)]),
            activities_projects: map(&[("erat tortor:ac", 42544)]),
            projects: map(&[("ac", 42544)]),
            tags: map(&[("metus", 42544), ("ut", 42544), ("sapien", 42544)])
        };

        let mut summary = new_summary();
        summary.comment = Some(String::from("curae"));

        assert_eq!(expected, summary.collect_statistics(&test::read_test_entries()));
    }

    #[test]
    fn test_limit() {
        let mut summary = new_summary();
        summary.from = None;
        summary.to = None;
        summary.all = true;
        summary.limit = 5;

        let stats = summary.collect_statistics(&test::read_test_entries());
        assert_eq!(stats.total, Duration::seconds(752098));
    }

    #[test]
    fn test_activity() {
        let mut summary = new_summary();
        summary.from = None;
        summary.to = None;
        summary.all = true;
        summary.activity_project_tags = vec![String::from("mauris")];

        let stats = summary.collect_statistics(&test::read_test_entries());

        assert_eq!(stats.total, Duration::seconds(3549273));

        for (activity, _) in stats.activities {
            assert_eq!(&activity, "mauris");
        }

        for (activity_project, _) in stats.activities_projects {
            assert!(&activity_project.starts_with("mauris"));
        }
    }

    #[test]
    fn test_activity_project() {
        let mut summary = new_summary();
        summary.from = None;
        summary.to = None;
        summary.all = true;
        summary.activity_project_tags = vec![String::from("eu"), String::from("interdum:nullam")];

        let expected = Statistics {
            total: Duration::seconds(86377),
            activities: map(&[("eu interdum", 86377)]),
            activities_projects: map(&[("eu interdum:nullam", 86377)]),
            projects: map(&[("nullam", 86377)]),
            tags: HashMap::new()
        };

        let stats = summary.collect_statistics(&test::read_test_entries());
        assert_eq!(stats, expected);
    }

    #[test]
    fn test_tags() {
        let mut summary = new_summary();
        summary.from = None;
        summary.to = None;
        summary.all = true;
        summary.activity_project_tags = vec![String::from("+mollis"), String::from("+ut")];

        let expected = Statistics {
            total: Duration::seconds(19363),
            activities: map(&[("platea", 19363)]),
            activities_projects: map(&[("platea:amet turpis", 19363)]),
            projects: map(&[("amet turpis", 19363)]),
            tags: map(&[("quisque", 19363), ("ut", 19363), ("molestie", 19363), ("lorem", 19363), ("mollis", 19363)])
        };

        let stats = summary.collect_statistics(&test::read_test_entries());
        assert_eq!(stats, expected);
    }

    #[test]
    fn test_all() {
        // The other relative period arguments (-y, -w, -d) can't be tested
        // since they're relative to the current day and would fail on any other day.

        let mut summary = new_summary();
        summary.from = None;
        summary.to = None;
        summary.all = true;

        let stats = summary.collect_statistics(&test::read_test_entries());

        // We just pick a few entries to assert here, otherwise the assertion
        // would span thousands of lines.
        assert_eq!(stats.total, Duration::seconds(806924147));
        assert_eq!(stats.activities.len(), 840);
        assert_eq!(stats.activities.iter().fold(Duration::zero(), |acc, x| acc + *x.1), Duration::seconds(806924147));
        assert_eq!(stats.activities_projects.len(), 2064);
        assert_eq!(stats.projects.len(), 703);
        assert_eq!(*stats.projects.get("felis ut").unwrap(), Duration::seconds(413035));
        assert_eq!(*stats.projects.get("morbi porttitor").unwrap(), Duration::seconds(295106));
        assert_eq!(stats.tags.len(), 168);
        assert_eq!(*stats.tags.get("pellentesque").unwrap(), Duration::seconds(12448673));
        assert_eq!(*stats.tags.get("sem").unwrap(), Duration::seconds(6927530));
        assert_eq!(*stats.tags.get("mollis").unwrap(), Duration::seconds(397752));
    }
}
