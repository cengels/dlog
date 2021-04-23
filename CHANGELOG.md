# Changelog

All items marked as "**changed**", "**removed**", or "**renamed**" are likely to
be breaking changes. Before 1.0.0, these changes will **not be announced** and
will only be documented in this file, so please make sure to read it thoroughly
if you wish to upgrade.

## [0.1.2](https://github.com/cengels/dlog/compare/v0.1.1...v0.1.2) - 2021-04-23

- **Added**: option `-d`/`--duration` now accepts a temporal expression similar
  to `-t`/`--to` and `-f`/`--from`
- **Added**: option `-n`/`--new` on `fill` forces dlog to always create a new entry,
  even if the content of the entry is identical to the last entry
- **Fixed**: output for `dlog fill` showed an incorrect added duration when used
  on an incomplete entry created through `dlog start`
- **Fixed**: *no such file* error when initially running any command while the
  `entries` file has not yet been created

## [0.1.1](https://github.com/cengels/dlog/compare/v0.1.0...v0.1.1) - 2021-01-14

- **Added**: identical entries during `dlog fill` now automatically merge
  with each other as if `dlog fill -u` were called
  ([commit](https://github.com/cengels/dlog/commit/09e167a487bfb8f2c0ccb46707760afc083096f5))
- **Fixed**: display of negative delta durations for `dlog fill -u`
  ([commit](https://github.com/cengels/dlog/commit/feffdcfc9e21ef942d4acf472de515b56cd0a908))
- **Fixed**: `dlog summary` not converting to UTC time correctly when using the flags
  `-d`, `-w`, `-y`, and `-a`.
  ([commit](https://github.com/cengels/dlog/commit/41ce60ab9a82d55e26492181c5fd2256e89ab72d))
- **Fixed**: comment filter not working for `dlog summary -c <comment>`
  ([commit](https://github.com/cengels/dlog/commit/28892b96a36915e9b6cc79799469b44b0da49260))
- **Fixed**: `dlog start` followed by `dlog fill` no longer causes an error
  ([commit](https://github.com/cengels/dlog/commit/af16dd2385a507c520dc88a4366c3c9a8b8278c6))

## [0.1.0](https://github.com/cengels/dlog/releases/tag/v0.1.0) - 2020-12-18

- Initial release
