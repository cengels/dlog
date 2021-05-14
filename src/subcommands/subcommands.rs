use std::error::Error;
use clap::{Clap, crate_version, crate_description};
use enum_dispatch::enum_dispatch;
use super::{Fill, Log, Remove, Start, Summary, Count, GenerateCompletions};

#[enum_dispatch]
pub trait Subcommand {
    /// Runs the subcommand with the parsed arguments.
    fn run(&self) -> Result<(), Box<dyn Error>>;
}

#[enum_dispatch(Subcommand)]
#[derive(Clap)]
#[clap(version = crate_version!(), about = crate_description!())]
pub enum Subcommands {
    Log(Log),
    Start(Start),
    Fill(Fill),
    Remove(Remove),
    Count(Count),
    Summary(Summary),
    GenerateCompletions(GenerateCompletions)
}
