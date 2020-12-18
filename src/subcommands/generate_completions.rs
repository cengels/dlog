use std::{error::Error, io};
use clap::{Clap, IntoApp};
use clap_generate::{generate, generators::*};
use super::{Subcommand, Subcommands};

/// Generates auto-completions for a given shell.
#[derive(Clap)]
pub struct GenerateCompletions {
    #[clap(possible_values = &["bash", "zsh", "fish", "powershell", "elvish"])]
    shell: String
}

impl Subcommand for GenerateCompletions {
    fn run(&self) -> Result<(), Box<dyn Error>> {
        let mut app = Subcommands::into_app();

        match self.shell.as_str() {
            "bash" => generate::<Bash, _>(&mut app, "completions", &mut io::stdout()),
            "zsh" => generate::<Zsh, _>(&mut app, "completions", &mut io::stdout()),
            "fish" => generate::<Fish, _>(&mut app, "completions", &mut io::stdout()),
            "powershell" => generate::<PowerShell, _>(&mut app, "completions", &mut io::stdout()),
            "elvish" => generate::<Elvish, _>(&mut app, "completions", &mut io::stdout()),
            _ => return Err(clap::Error::with_description(String::from("Invalid value."), clap::ErrorKind::InvalidValue).into())
        }

        Ok(())
    }
}
