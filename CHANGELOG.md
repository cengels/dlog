# Changelog

All items marked as "**changed**", "**removed**", or "**renamed**" are likely to
be breaking changes. Before 1.0.0, these changes will **not be announced** and
will only be documented in this file, so please make sure to read it thoroughly
if you wish to upgrade.

## [Upcoming](https://github.com/cengels/dlog/compare/v0.1.0...HEAD)

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

## [0.1.0](https://github.com/cengels/dlog/releases/tag/v0.1.0) - 2020-12-18

- Initial release
