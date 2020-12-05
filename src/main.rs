use clap::Clap;
use subcommands::Subcommand;

#[path = "subcommands/subcommands.rs"]
mod subcommands;

fn main() {
    let subcommand: subcommands::Subcommands = subcommands::Subcommands::parse();
    subcommand.run();
}
