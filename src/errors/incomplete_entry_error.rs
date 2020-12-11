#[derive(Debug)]
pub struct IncompleteEntryError;

impl std::fmt::Display for IncompleteEntryError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str("The last entry is incomplete. Please complete it first before starting another incomplete entry.")
    }
}

impl std::error::Error for IncompleteEntryError {}
