#include <iomanip>
#include <experimental/filesystem>
#include <ostream>
#include <fstream>
#include "version.h"
#include "../cli/color.h"
#include "../entries.h"
#include "../format.h"
#include "../parser.h"
#include "../commandline.h"
#include "../files.h"
#include "subcommands.h"
#include "summary.h"

namespace {
    struct summary_results
    {
        std::map<std::string, time_t> activities;
        std::map<std::string, time_t> projects;
        std::map<std::string, time_t> activity_projects;
        std::map<std::string, time_t> tags;
        time_t total_time = 0;
    };

    constexpr int FIELD_WIDTH = 60;
    constexpr int DURATION_WIDTH = 20;
}

cxxopts::Options subcommands::summary::options() const
{
    auto opts = subcommands::subcommand::options();

    opts.add_options()
        ("h,help", "Prints all available options.")
        ("s,string", "Only summarize entries where this string can be found in its activity, project, tags, or comment.", cxxopts::value<std::string>()->default_value(std::string()))
        ("a,activity", "Only summarize entries with this activity.", cxxopts::value<std::string>()->default_value(std::string()))
        ("p,project", "Only summarize entries with this project.", cxxopts::value<std::string>()->default_value(std::string()))
        ("t,tag", "Only summarize entries containing this tag. Can be specified multiple times.", cxxopts::value<std::vector<std::string>>()->default_value(std::string()))
        ("c,comment", "Only summarize entries with this substring in its comment.", cxxopts::value<std::string>()->default_value(std::string()))
        ("f,from", "Only summarize entries after this date-time string.", cxxopts::value<std::string>()->default_value("30 days ago"))
        ("to", "Only summarize entries before this date-time string.", cxxopts::value<std::string>()->default_value(std::string()))
        ("l,limit", "Only summarize the last n entries matching the filters.", cxxopts::value<int>()->default_value("0"))
        ("P,no-pager", "Directs output to the standard output instead of the default pager.");

    return opts;
}

static bool validate(const time_t& from, const time_t& to, int limit)
{
    if (from == 0) {
        std::cout << "Invalid datetime for -f. Use -h for help." << std::endl;
        return false;
    }

    if (to == 0) {
        std::cout << "Invalid datetime for -t. Use -h for help." << std::endl;
        return false;
    }

    if (from >= to) {
        std::cout << "Invalid time frame. Start must come before end." << std::endl;
        return false;
    }

    if (limit < 0) {
        std::cout << "Limit (-l) must be positive." << std::endl;
        return false;
    }

    return true;
}

static bool compare_strings(const std::string& target, const std::string& filter)
{
    return filter.empty() || target == filter;
}

static bool contains(const std::string& target, const std::string& filter)
{
    return filter.empty() || target.find(filter) != std::string::npos;
}

static bool contains(const std::vector<std::string>& target, const std::string& filter, bool partial_matches = false)
{
    if (filter.empty()) {
        return true;
    }

    if (partial_matches) {
        for (const auto& string : target) {
            if (contains(string, filter)) {
                return true;
            }
        }

        return false;
    }

    return std::find(target.cbegin(), target.cend(), filter) != target.cend();
}

static bool contains(const std::vector<std::string>& target, const std::vector<std::string>& filter, bool partial_matches = false)
{
    if (filter.empty()) {
        return true;
    }

    for (const std::string& string : filter) {
        if (!contains(target, string, partial_matches)) {
            return false;
        }
    }

    return true;
}

static std::vector<entries::entry> apply_filter(const std::vector<entries::entry>& entries, entries::entry& filter_entry, const std::string& string, int limit)
{
    std::vector<entries::entry> results(limit == 0 ? 30 : limit);
    int out_of_bounds_entries = 0;

    for (const entries::entry& entry : entries) {
        if (!entry.complete()) {
            continue;
        }

        if (entry.to > filter_entry.to) {
            continue;
        }

        if (entry.from < filter_entry.from) {
            out_of_bounds_entries++;

            if (out_of_bounds_entries > 7) {
                // Ideally the entries list should be sorted chronologically
                // so if there is an entry that is further back than -f,
                // we could end immediately. However, we give it a "grace period"
                // of 7 entries in case the sorting is messed up slightly.
                return results;
            }

            continue;
        }

        if (!compare_strings(entry.activity, filter_entry.activity)
         || !compare_strings(entry.project, filter_entry.project)
         || !contains(entry.tags, filter_entry.tags)
         || !contains(entry.comment, filter_entry.comment)) {
            continue;
        }

        if (!string.empty()
         && !contains(entry.activity, string)
         && !contains(entry.project, string)
         && !contains(entry.comment, string)
         && !contains(entry.tags, string, true)) {
             continue;
        }

        results.push_back(entry);

        if (limit > 0 && results.size() >= limit) {
            return results;
        }
    }

    return results;
}

static void increment(std::map<std::string, time_t>& map, const std::string& key, time_t duration)
{
    map.insert_or_assign(key, map.count(key) != 0 ? map[key] + duration : duration);
}

static summary_results summarize(const std::vector<entries::entry>& entries)
{
    summary_results results;

    for (const entries::entry& entry : entries) {
        increment(results.activities, entry.activity, entry.to - entry.from);

        if (!entry.project.empty()) {
            increment(results.projects, entry.project, entry.to - entry.from);
            std::string activity_project = entry.activity;
            activity_project.append(":");
            activity_project.append(entry.project);
            increment(results.activity_projects, activity_project, entry.to - entry.from);
        }

        for (const std::string& tag : entry.tags) {
            increment(results.tags, tag, entry.to - entry.from);
        }

        results.total_time += (entry.to - entry.from);
    }

    return results;
}

static void print_map(std::ostream& stream, const std::map<std::string, time_t>& map, const cli::color& color)
{
    for (const auto& item : map) {
        if (item.first.empty()) {
            continue;
        }

        stream << "    " << color << std::setw(FIELD_WIDTH) << item.first
               << cli::color::reset << " "
               << cli::color::duration << std::right << std::setw(DURATION_WIDTH) << format::as_duration(item.second, true)
               << std::left << cli::color::reset << "\n";
    }
}

static void print_results(std::ostream& stream, const summary_results& results, const entries::entry& filter_entry)
{
    stream << std::left;

    stream << "Summary of entries between "
           << cli::color::time << format::as_local_time_string(filter_entry.from)
           << cli::color::reset << " and " << cli::color::time << format::as_local_time_string(filter_entry.to)
           << cli::color::reset << ":\n\n  " << std::setw(FIELD_WIDTH + 3) << "Total time spent: "
           << cli::color::duration << std::right << std::setw(DURATION_WIDTH) << format::as_duration(results.total_time, true)
           << std::left << cli::color::reset << "\n\n";

    stream << "  Activities:\n\n";

    print_map(stream, results.activities, cli::color::activity);

    for (const auto& activity_project : results.activity_projects) {
        int divider_position = activity_project.first.find(':');
        std::string activity = activity_project.first.substr(0, divider_position);
        std::string project = activity_project.first.substr(divider_position + 1);
        stream << "    " << cli::color::activity << activity << cli::color::reset
                  << ":" << cli::color::project << std::setw(FIELD_WIDTH - activity.size()) << project
                  << cli::color::reset << " " << cli::color::duration << std::right
                  << std::setw(DURATION_WIDTH) << format::as_duration(activity_project.second, true)
                  << std::left << cli::color::reset << "\n";
    }

    if (!results.projects.empty()) {
        stream << "\n  Projects:\n\n";

        print_map(stream, results.projects, cli::color::project);
    }

    if (!results.tags.empty()) {
        stream << "\n  Tags:\n\n";

        print_map(stream, results.tags, cli::color::tag);
    }

    stream << std::endl;
}

int subcommands::summary::run(const cxxopts::ParseResult& parsedOptions)
{
    if (parsedOptions.count("help") != 0) {
        return subcommands::subcommand::run(parsedOptions);
    }

    entries::entry filter_entry;

    const std::string string = parsedOptions["s"].as<std::string>();
    filter_entry.activity = parsedOptions["a"].as<std::string>();
    filter_entry.project = parsedOptions["p"].as<std::string>();
    filter_entry.tags = parsedOptions["t"].as<std::vector<std::string>>();
    filter_entry.comment = parsedOptions["c"].as<std::string>();
    const std::string from_string = parsedOptions["f"].as<std::string>();
    const std::string to_string = parsedOptions["to"].as<std::string>();
    filter_entry.from = parser::parse_temporal(from_string);
    filter_entry.to = to_string.empty() ? std::time(nullptr) : parser::parse_temporal(to_string);
    int limit = parsedOptions["l"].as<int>();

    if (!validate(filter_entry.from, filter_entry.to, limit)) {
        return 1;
    }

    const std::vector<entries::entry> entries = entries::read_all();
    const std::vector<entries::entry> filtered_entries = apply_filter(entries, filter_entry, string, limit);
    const summary_results summary_results = summarize(filtered_entries);

    if (summary_results.total_time == 0) {
        std::cout << "No entries found between "
                  << cli::color::time << format::as_local_time_string(filter_entry.from)
                  << cli::color::reset << " and "
                  << cli::color::time << format::as_local_time_string(filter_entry.to)
                  << cli::color::reset << std::endl;

        return 0;
    }

    if (parsedOptions.count("no-pager") == 0) {
        auto path = std::experimental::filesystem::temp_directory_path();

        if (!path.empty()) {
            path.append("dlog_summary_output");
            std::ofstream stream(path.c_str(), std::ios_base::trunc);
            rang::setControlMode(rang::control::Force);
            print_results(stream, summary_results, filter_entry);
            rang::setControlMode(rang::control::Auto);
            stream.close();

            files::open_in_pager(path);
            std::experimental::filesystem::remove(path);

            return 0;
        }
    }

    print_results(std::cout, summary_results, filter_entry);

    return 0;
}

std::string subcommands::summary::command() const
{
    return "summary";
}

std::string subcommands::summary::syntax() const
{
    return "[-h] [-s <string>] [-a <activity>] [-p <project>] [-t <tag>...]\n"
           "               "
           "[-c <comment>] [-f <datetime>] [--to <datetime>] [-l <limit>]";
}

std::string subcommands::summary::description() const
{
    return "Finds the total time spent on certain activities or projects within a given time frame.\n"
           "  You can specify multiple filters to further narrow down the result.\n"
           "  If no time frame is specified, shows the last thirty days.\n\n"
           "  Examples:\n"
           "    dlog summary -a gaming -f \"1 week ago\"  (how much did I play games in the past week?)\n"
           "    dlog summary -t healthy -t restaurant  (how much healthy food did I eat at restaurants in the past 30 days?)";
}
