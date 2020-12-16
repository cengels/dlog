#[derive(Debug)]
pub struct InvalidEntryError;

impl std::fmt::Display for InvalidEntryError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str("Invalid entry. The activity must not be empty and the start point must be smaller than the end point.")
    }
}

impl std::error::Error for InvalidEntryError {}
