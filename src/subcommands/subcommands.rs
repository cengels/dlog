use clap::Clap;
use enum_dispatch::enum_dispatch;
use super::Log;

#[enum_dispatch]
pub trait Subcommand {
    /// Runs the subcommand with the parsed arguments.
    fn run(&self);
}

#[enum_dispatch(Subcommand)]
#[derive(Clap)]
#[clap()]
pub enum Subcommands {
    Log(Log)
}
