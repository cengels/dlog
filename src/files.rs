use std::{io::ErrorKind, path::PathBuf};

const DLOG_ENV_VAR: &str = "DLOG_PATH";

fn base_path() -> Result<PathBuf, std::io::Error> {
    if let Ok(path) = std::env::var(DLOG_ENV_VAR) {
        return Ok(PathBuf::from(path));
    }

    let base: Result<PathBuf, std::io::Error> = dirs::data_dir()
        .ok_or_else(|| std::io::Error::new(ErrorKind::NotFound, "Could not locate appropriate dlog directory. This is most likely not your fault. To fix this problem, try defining the path dlog should use to store your entries explicitly in the environment variable DLOG_PATH."));

    let mut path = base?;

    path.push("dlog");

    if !path.is_dir() {
        std::fs::create_dir(path.clone())?;
    }

    Ok(path)
}

/// Attempts to find the appropriate file path for the specified file
/// in dlog's own data directory.
pub fn path(file: &str) -> Result<PathBuf, std::io::Error> {
    let mut path = base_path()?;

    path.push(file);

    Ok(path)
}

/// Attempts to find the appropriate temporary file path for the specified file
/// in dlog's own data directory, used to write a file before replacing the real file with it.
pub fn tmp_path(file: &str) -> Result<PathBuf, std::io::Error> {
    let mut path = path(file)?;

    path.set_extension("tmp");

    Ok(path)
}
