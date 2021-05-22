#![allow(dead_code)]
#![warn(clippy::all, clippy::pedantic, clippy::nursery, clippy::cargo)]
#![allow(clippy::module_inception,  // incorrectly barks on declaring mod subcommands in mod.rs
    clippy::struct_excessive_bools,  // necessary due to Clap
    clippy::module_name_repetitions,  // when imported unprefixed, this would cause potential confusion
    clippy::map_err_ignore,  // ignoring errors is perfectly acceptable when there is an alternative course of action (see parser.rs)
    clippy::wildcard_imports,  // necessary for mod.rs files
    clippy::multiple_crate_versions)]  // unfixable as those crates are transitive dependencies

use clap::Clap;
use colored::*;
use subcommands::Subcommand;

mod data;
mod subcommands;
mod entries;
mod files;
mod errors;
mod format;
mod input;

#[cfg(test)]
mod test;

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
