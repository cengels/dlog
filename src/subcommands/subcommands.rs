use std::error::Error;
use clap::Clap;
use enum_dispatch::enum_dispatch;
use super::{Log, Start};

#[enum_dispatch]
pub trait Subcommand {
    /// Runs the subcommand with the parsed arguments.
    fn run(&self) -> Result<(), Box<dyn Error>>;
}

#[enum_dispatch(Subcommand)]
#[derive(Clap)]
#[clap()]
pub enum Subcommands {
    Log(Log),
    Start(Start)
}
