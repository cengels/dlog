#[derive(Debug)]
pub struct NotImplementedError;

impl std::fmt::Display for NotImplementedError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str("This function is not implemented yet.")
    }
}

impl std::error::Error for NotImplementedError {}
