use super::{ENTRY_FILE, Entry};
use crate::files;

pub fn rewrite(entries: &[Entry]) -> Result<(), std::io::Error> {
    let path = files::path(ENTRY_FILE)?;
    let tmp_path = files::tmp_path(ENTRY_FILE)?;
    let mut writer = csv::WriterBuilder::new()
        .has_headers(false)
        .double_quote(false)
        .quote_style(csv::QuoteStyle::NonNumeric)
        .from_path(&tmp_path)?;

    for entry in entries {
        writer.serialize(entry)?;
    }

    std::fs::rename(&tmp_path, &path)
}
