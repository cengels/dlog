#![allow(dead_code)]
#![allow(clippy::module_inception)]

use clap::Clap;
use colored::*;
use subcommands::Subcommand;

mod subcommands;
mod entries;
mod files;
mod errors;
mod format;
mod parser;

fn main() {
    std::panic::set_hook(Box::new(|info| {
        let msg = match info.payload().downcast_ref::<&'static str>() {
            Some(s) => *s,
            None => match info.payload().downcast_ref::<String>() {
                Some(s) => &s[..],
                None => "an unknown error has occurred.",
            }
        };

        // The broken pipe error occurs when using the pager crate.
        // It isn't fixable without implementing a custom pager implementation,
        // but that's okay because the program is set to end after
        // the pager executes anyway.
        if !msg.contains("Broken pipe") {
            eprintln!("{}: {}", "error".red(), msg.trim_start_matches("error: "));
        }
    }));

    // Necessary to ensure using the pager still prints color.
    // Probably doesn't work in a Windows shell. Testing required.
    std::env::set_var("CLICOLOR_FORCE", "1");

    let subcommand: subcommands::Subcommands = subcommands::Subcommands::parse();

    if let Err(error) = subcommand.run() {
        eprintln!("{}: {}", "error".red(), error.to_string().trim_start_matches("error: "));
    }
}
