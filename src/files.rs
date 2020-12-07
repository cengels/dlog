use dirs;
use std::{io::ErrorKind, path::PathBuf};

const DLOG_ENV_VAR: &str = "DLOG_PATH";

fn base_path() -> Result<PathBuf, std::io::Error> {
    if let Ok(path) = std::env::var(DLOG_ENV_VAR) {
        return Ok(PathBuf::from(path));
    }

    let base: Result<PathBuf, std::io::Error> =
        if let Some(data_dir) = dirs::data_dir() {
            Ok(data_dir)
        } else {
            Err(std::io::Error::new(ErrorKind::NotFound, "Could not locate appropriate dlog directory. This is most likely not your fault. To fix this problem, try defining the path dlog should use to store your entries explicitly in the environment variable DLOG_PATH."))
        };

    let mut path = base?;

    path.push("dlog");

    if !path.is_dir() {
        std::fs::create_dir(path.clone())?;
    }

    Ok(path)
}

pub fn path(file: &str) -> Result<PathBuf, std::io::Error> {
    let mut path = base_path()?;

    path.push(file);

    Ok(path)
}
