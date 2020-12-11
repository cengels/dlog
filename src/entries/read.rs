use std::error::Error;

use super::{ENTRY_FILE, Entry};
use crate::files;

pub fn read_all() -> Result<Vec<Entry>, Box<dyn Error>> {
    let path = files::path(ENTRY_FILE)?;
    let mut reader = csv::ReaderBuilder::new()
        .has_headers(false)
        .from_path(path)?;

    reader.deserialize()
        .map(|x| -> Result<Entry, Box<dyn Error>> { Ok(x?) })
        .collect()
}
