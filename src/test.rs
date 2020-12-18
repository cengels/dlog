use std::path::PathBuf;
use chrono::{DateTime, Duration, NaiveDate, Utc};
use proptest::{collection::vec, prelude::*};
use crate::entries::{self, Entry};

pub fn read_test_entries() -> Vec<Entry> {
    let entries = entries::read_all_from(&PathBuf::from("./test_entries.csv")).unwrap();

    assert_eq!(entries.len(), 3000);

    entries
}

prop_compose! {
    pub fn generate_datetime()(
        year in 1970..3000i32,
        day in 1..365u32,
        hour in 0..23u32,
        minute in 0..59u32,
        second in 0..59u32
    ) -> DateTime<Utc> {
        DateTime::<Utc>::from_utc(NaiveDate::from_yo(year, day).and_hms(hour, minute, second), Utc)
    }
}

prop_compose! {
    pub fn generate_entry()(
        activity in ".*",
        project in ".*",
        from in generate_datetime(),
        to in generate_datetime(),
        tags in vec(".+", 0..9),
        comment in ".*"
    ) -> Entry {
        Entry {
            activity,
            project,
            from,
            to,
            tags,
            comment
        }
    }
}

prop_compose! {
    pub fn generate_valid_entry()(mut entry in generate_entry()) -> Entry {
        if entry.to > Utc::now() {
            // Ensure that to cannot be in the future.
            entry.to = Utc::now();
        }

        if entry.from > Utc::now() {
            // Ensure that from cannot be in the future
            // and cannot coincide with Utc::now().
            entry.from = Utc::now() - Duration::seconds(1);
        }

        if entry.to < entry.from {
            let tmp = entry.from;
            entry.to = entry.from;
            entry.from = tmp;
        } else if entry.to == entry.from {
            entry.to = entry.to + Duration::seconds(1);
        }
        
        entry
    }
}

prop_compose! {
    pub fn generate_complete_entry()(mut entry in generate_valid_entry(), activity in ".+") -> Entry {
        entry.activity = activity;

        if entry.from.timestamp() == 0 {
            // Ensure that from is considered "set".
            entry.from = entry.from + Duration::seconds(1);
        }

        if entry.to <= entry.from {
            entry.to = entry.from + Duration::seconds(1);
        }
        
        entry
    }
}
