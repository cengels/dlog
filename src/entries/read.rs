use std::{error::Error, path::PathBuf};

use super::{ENTRY_FILE, Entry};
use crate::files;

/// Attempts to read all entries from an automatically located
/// entries file.
pub fn read_all() -> Result<Vec<Entry>, Box<dyn Error>> {
    let path = files::path(ENTRY_FILE)?;
    read_all_from(&path)
}

/// Attempts to read all entries from the specified path.
pub fn read_all_from(path: &PathBuf) -> Result<Vec<Entry>, Box<dyn Error>> {
    let mut reader = csv::ReaderBuilder::new()
        .has_headers(false)
        .from_path(path)?;

    reader.deserialize()
        .map(|x| -> Result<Entry, Box<dyn Error>> { Ok(x?) })
        .collect()
}


#[cfg(test)]
mod test {
    use std::path::PathBuf;

    use super::read_all_from;

    #[test]
    fn test_read_missing_file() {
        assert!(read_all_from(&PathBuf::from("./random_probably_non_existent_path")).is_err());
    }

    #[test]
    fn test_read_file() {
        let result = read_all_from(&PathBuf::from("./test_entries.csv"));
        assert_eq!(result.unwrap().len(), 3000);
    }
}
