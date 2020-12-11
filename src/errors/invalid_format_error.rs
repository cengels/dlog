#[derive(Debug)]
pub struct InvalidFormatError(pub String);

impl std::fmt::Display for InvalidFormatError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Invalid format. Value must be in the format of {}", self.0)
    }
}

impl std::error::Error for InvalidFormatError {}
