use std::{cmp::Ordering, collections::HashMap, vec::IntoIter};
use chrono::Duration;
use colored::{ColoredString, Colorize};

pub const DURATION_WIDTH: usize = 30;
pub const FIELD_WIDTH: usize = 50;

#[derive(Debug, PartialEq)]
pub struct Statistics<T>
  where T: PartialOrd {
    pub total: T,
    pub activities: HashMap<String, T>,
    pub activities_projects: HashMap<String, T>,
    pub projects: HashMap<String, T>,
    pub tags: HashMap<String, T>
}

impl<T> Statistics<T>
  where T: PartialOrd {
    pub fn print<F>(&self, callback: F)
      where F: Fn(&T) -> ColoredString {
        println!("Activities:\n");

        for activity in sort(&self.activities) {
            println!("{:w$} {:>dw$}", activity.0.cyan(), callback(activity.1), w = FIELD_WIDTH, dw = DURATION_WIDTH);
        }

        for activity_project in sort(&self.activities_projects) {
            let colon_index = activity_project.0.find(':').unwrap();
            let string = format!("{}:{}", activity_project.0[0..colon_index].cyan(), activity_project.0[colon_index + 1..].bright_red());
            // Due to the ANSI escape code a higher field width is required here.
            println!("{:w$} {:>dw$}", string, callback(activity_project.1), w = FIELD_WIDTH + 18, dw = DURATION_WIDTH);
        }

        if !self.projects.is_empty() {
            println!("\nProjects:\n");

            for project in sort(&self.projects) {
                println!("{:w$} {:>dw$}", project.0.bright_red(), callback(project.1), w = FIELD_WIDTH, dw = DURATION_WIDTH);
            }
        }

        if !self.tags.is_empty() {
            println!("\nTags:\n");

            for tag in sort(&self.tags) {
                println!("{:w$} {:>dw$}", tag.0.bright_yellow(), callback(tag.1), w = FIELD_WIDTH, dw = DURATION_WIDTH);
            }
        }
    }
}

impl Statistics<Duration> {
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

impl Statistics<u32> {
    pub fn new() -> Self {
        Self {
            total: 0,
            activities: HashMap::new(),
            activities_projects: HashMap::new(),
            projects: HashMap::new(),
            tags: HashMap::new()
        }
    }
}

fn sort<T>(map: &HashMap<String, T>) -> IntoIter<(&std::string::String, &T)>
  where T: PartialOrd {
    let mut vector = map.iter().collect::<Vec<(&String, &T)>>();
    vector.sort_by(|a, b| b.1.partial_cmp(a.1).unwrap_or(Ordering::Equal));

    vector.into_iter()
}
