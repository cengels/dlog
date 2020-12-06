use clap::Clap;
use subcommands::Subcommand;

mod subcommands;
mod entries;

fn main() {
    let subcommand: subcommands::Subcommands = subcommands::Subcommands::parse();
    subcommand.run();
}
