use std::error::Error;

use super::Entry;
use crate::files;

const ENTRY_FILE: &str = "entries";

pub fn read_all() -> Result<Vec<Entry>, Box<dyn Error>> {
    let path = files::path(ENTRY_FILE)?;
    let mut reader = csv::ReaderBuilder::new()
        .has_headers(false)
        .from_path(path)?;

    reader.deserialize()
        // .map(|x| -> Result<ResultTuple, Box<dyn Error>> { Ok(x?) })
        .map(|x| -> Result<Entry, Box<dyn Error>> { Ok(x?) })
        .collect()
}
