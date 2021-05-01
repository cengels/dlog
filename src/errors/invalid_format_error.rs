#[derive(Debug)]
pub struct InvalidFormatError {
    expression: String,
    message: String,
    /// Whether the format was at least partially matched.
    pub partial_match: bool
}

impl std::fmt::Display for InvalidFormatError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "No matching format found for \"{}\". {}", self.expression, self.message)
    }
}

impl std::error::Error for InvalidFormatError {}

const MAX_FORMAT_WIDTH: usize = 38;

impl InvalidFormatError {
    pub fn duration(expression: &str) -> Self {
        Self {
            expression: expression.to_owned(),
            message: String::from("Value must be a valid duration in the format of [WW:][DD:][HH:]MM:SS or a temporal expression in the format of \"[n] [unit(s)]\" where unit is one of: seconds, hours, days, weeks, months, years (or their singular counterpart)."),
            partial_match: false
        }
    }

    pub fn invalid_number_with_bounds(number: &str, bounds: i64) -> Self {
        Self {
            expression: number.to_owned(),
            message: format!("Value must be a valid positive number under {}.", bounds),
            partial_match: true
        }
    }

    pub fn invalid_number(number: &str) -> Self {
        Self {
            expression: number.to_owned(),
            message: String::from("Value must be a valid positive number."),
            partial_match: true
        }
    }

    pub fn temporal_expression(expression: &str) -> Self {
        Self {
            expression: expression.to_owned(),
            message: String::from("Value must be a valid temporal expression in the format of \"in [n] [unit(s)]\" or \"[n] [unit(s)] ago\" where unit is one of: seconds, hours, days, weeks, months, years (or their singular counterpart)."),
            partial_match: false
        }
    }

    pub fn datetime(expression: &str) -> Self {
        Self {
            expression: expression.to_owned(),
            message: format!("Valid formats include:\n{:>w$} ({})\n{:>w$} ({})\n{:>w$} ({})\n{:>w$} ({})\n{:>w$}\n{:>w$}",
                "Thu, 16 Jul 2020 16:05:32 +0100", "RFC 2822",
                "2020-06-16T16:05:32+01:00", "RFC 3339 / ISO 8601",
                "16:05:32", "recommended time format",
                "2020-06-16 16:05:32", "recommended date format",
                "2 weeks ago",
                "in 8 hours",
                w = MAX_FORMAT_WIDTH
            ),
            partial_match: false
        }
    }
}
