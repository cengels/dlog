use clap::Clap;
use super::Subcommand;

#[derive(Clap)]
pub struct Log {
    #[clap(short = 'P', long)]
    no_pager: bool,
    #[clap(short, long)]
    comments: bool,
    #[clap(short, long, default_value = "0")]
    limit: u32
}

impl Subcommand for Log {
    fn run(&self) {
        todo!();
    }
}
