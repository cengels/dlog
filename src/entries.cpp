#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include "files.h"
#include "entries.h"

namespace {
    std::experimental::filesystem::path m_entries_file_path;
}

std::experimental::filesystem::path& entries_file_path()
{
    if (m_entries_file_path.empty()) {
        m_entries_file_path = files::dlog_file("entries");
    }

    return m_entries_file_path;
}

void entries::serialize(std::ostream& stream, const entries::entry& entry)
{
    if (!entry.valid()) {
        return;
    }

    stream << entry.from << ","
           << entry.to << ",\""
           << entry.activity << "\",\""
           << entry.project << "\",\"";

    const auto& last = entry.tags.back();

    for (const std::string& tag : entry.tags) {
        stream << tag;

        if (tag != last) {
            stream << ",";
        }
    }

    stream << "\"";
}

static entries::entry parse(const std::string& line)
{
    if (line.empty()) {
        return entries::entry();
    }

    entries::entry entry;

    int position_start = 0;
    int position_end = line.find(',');
    entry.from = std::stol(line.substr(0, position_end));

    position_start = position_end + 1;
    position_end = line.find(',', position_start);
    entry.to = std::stol(line.substr(position_start, position_end - position_start));

    position_start = position_end + 2;
    position_end = line.find('"', position_start);
    entry.activity = line.substr(position_start, position_end - position_start);

    position_start = position_end + 3;
    position_end = line.find('"', position_start);
    entry.project = line.substr(position_start, position_end - position_start);

    position_start = position_end + 3;
    position_end = line.find('"', position_start);
    std::string tags = line.substr(position_start, position_end - position_start);

    position_start = 0;
    position_end = tags.find(',');
    const int length = tags.length();

    while (position_start < length) {
        if (position_end == std::string::npos) {
            position_end = length;
        }

        const std::string tag = tags.substr(position_start, position_end - position_start);

        if (!tag.empty()) {
            entry.tags.push_back(tag);
        }

        position_start = position_end + 1;
        position_end = tags.find(',', position_start);
    }

    return entry;
}

entries::entry entries::last()
{
    const std::experimental::filesystem::path& file_path = entries_file_path();

    if (file_path.empty()) {
        return entries::entry();
    }

    std::ifstream entries_file(file_path.c_str());
    std::string last_line;
    files::get_last_nonempty_line(entries_file, last_line);

    entries_file.close();

    return parse(last_line);
}

bool entries::write(const entries::entry& entry)
{
    if (!entry.valid()) {
        return false;
    }

    const std::experimental::filesystem::path& file_path = entries_file_path();

    if (file_path.empty()) {
        return false;
    }

    if (!files::prepare_for_write(file_path)) {
        return false;
    }

    std::fstream entries_file(file_path.c_str(), std::ios::in | std::ios::out);
    files::append_to_last_line(entries_file);

    entries::serialize(entries_file, entry);
    entries_file << "\n";

    if (entries_file.fail()) {
        files::restore(file_path);
        return false;
    }

    files::accept_changes(file_path);
    return true;
}

bool entries::overwrite_last(const entries::entry& entry)
{
    if (!entry.valid()) {
        return false;
    }

    const std::experimental::filesystem::path& file_path = entries_file_path();

    if (file_path.empty()) {
        return false;
    }

    if (!files::prepare_for_write(file_path)) {
        return false;
    }

    std::fstream entries_file(file_path.c_str(), std::ios::in | std::ios::out);

    std::string line;
    while (files::get_previous_line(entries_file, line)) {
        if (!line.empty()) {
            // Because files::get_previous_line() leaves the
            // stream position at the end of the line before the line written to result,
            // we need to manually go back to the beginning of the resulting line first.

            char c;

            entries_file.get(c);

            if (c == '\r') {
                entries_file.get(c);
            }

            break;
        }
    }

    entries::serialize(entries_file, entry);
    entries_file << "\n";

    if (entries_file.fail()) {
        files::restore(file_path);
        return false;
    }

    files::accept_changes(file_path);
    return true;
}

std::vector<entries::entry> entries::read_all(uint limit)
{
    std::vector<entries::entry> entries;
    const std::experimental::filesystem::path& file_path = entries_file_path();

    if (file_path.empty()) {
        return entries;
    }

    std::ifstream entries_file(file_path.c_str());
    std::string line;
    uint i = 0;
    files::get_last_line(entries_file, line);

    do {
        if (line.empty()) {
            continue;
        }

        entries::entry entry = parse(line);

        if (entry.valid()) {
            entries.push_back(entry);
        }

        i++;
    } while (files::get_previous_line(entries_file, line) && (limit == 0 || i < limit));

    entries_file.close();

    return entries;
}
