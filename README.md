<h1 align="center">dlog</h1>

<p align="center">Inverted command line time tracking.</p>

### Not another time tracking application!

Yes. Another time tracking application. Rejoice, for all your time tracking needs will finally be satisfied.

### What makes this one different?

The mentality and core concept. In most time tracking applications you have to specify an activity or project
when you start tracking your time on it. In other words: you need to already know what you'll be doing *before* you're doing it.

In dlog, you don't need to specify an activity or project until you're ready to *stop* the entry.
The general flow, therefore, looks like this:

```bash
dlog start  # starts a new incomplete time entry
dlog fill shower  # after you're done showering
dlog fill breakfast:toast  # "breakfast" is the activity, "toast" is the project
dlog fill workout -m "Good workout today."
dlog fill checking emails
dlog fill coding:dlog +rust +tests  # "rust" and "tests" are tags
# Oops! I took a break but forgot to track it!
dlog fill reddit --to "10 minutes ago"
dlog fill ...
```

This kind of workflow personally suits me better, but if you prefer to specify what you're doing when you start doing it
rather than when you stop, I recommend [watson](https://github.com/TailorDev/Watson).

### Features

- "inverted" time tracking (specify an activity when you stop tracking, not when you start)
- print a tabular history of all your past time entries (`dlog log`)
- print a (optionally filtered) summary showing you a summarized report of where you
  spend your time in a given time frame (`dlog summary`)
- blazingly fast performance even when dealing with tens of thousands of time entries

Feel free to open an issue if you'd like to request a new feature or improvement!

### Entry structure

Entries are stored in CSV (specify a custom directory by setting the environment variable `DLOG_PATH`)
and share the following structure:

```
timestamp_start, timestamp_end, activity, project, tags, message
```

The activity is the base component of each time entry, making it the only mandatory argument when using
`dlog fill`. If you generally think in terms of projects and not activities, it may help you to come up
with the most general activity you can think of for a set of projects. For instance, "work" as an activity
for all work-related projects. This allows you to efficiently group a set of entries, which is useful for
filtering in `dlog summary`.
