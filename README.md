# dlog
Configurable automated command line time tracking.
Note that this is a historical version of the program
and is no longer being actively developed. Switch to
[the master branch](https://github.com/cengels/dlog)
for the current version of the program, developed in Rust.

## History
This project first started out as a C++ project.
You can see the fruits of those labours here.
The program is fully compilable and executable
and has the same features as all master commits
up roughly to commit
[010da3a](https://github.com/cengels/dlog/commit/010da3a8dbdb315c22263f69030ccab54229a15a).
Some bugs were deliberately left untreated in this
branch in favour of the new Rust version.

Interestingly, the performance of the Rust version
is vastly superior to that of the C++ version.
This is likely to be due to inefficiencies in the
underlying algorithms, particularly in regards to
file reading. In Rust, those tasks are handled
by mature libraries specifically designed
for a singular purpose. In this version, I implemented
most of that functionality myself as I was unable
to find a suitable library in C++'s problematic
ecosystem. My implementation was likely subpar however.
