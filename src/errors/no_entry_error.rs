#[derive(Debug)]
pub struct NoEntryError;

impl std::fmt::Display for NoEntryError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str("No entries found. Call dlog start to create an incomplete entry\nthat can be used for dlog fill.")
    }
}

impl std::error::Error for NoEntryError {}
