#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "files.h"
#include "entries.h"

namespace fs = std::experimental::filesystem;

namespace {
    fs::path m_entries_file_path;
}

fs::path& entries_file_path()
{
    if (m_entries_file_path.empty()) {
        m_entries_file_path = files::dlog_file("entries");
    }

    return m_entries_file_path;
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

    position_start = position_end + 3;
    position_end = line.find('"', position_start);

    if (position_end != std::string::npos) {
        entry.comment = line.substr(position_start, position_end - position_start);
    }

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
    const fs::path& file_path = entries_file_path();

    if (file_path.empty()) {
        return entries::entry();
    }

    std::ifstream entries_file(file_path.c_str());
    std::string last_line;
    files::get_last_nonempty_line(entries_file, last_line);

    entries_file.close();

    return parse(last_line);
}

bool entries::append(const entries::entry& entry)
{
    if (!entry.valid()) {
        return false;
    }

    const fs::path& file_path = entries_file_path();

    if (file_path.empty()) {
        return false;
    }

    const fs::path& temp_path = files::prepare_for_write(file_path, true);

    if (temp_path.empty()) {
        return false;
    }

    std::fstream temp_file(temp_path.c_str(), std::ios::in | std::ios::out);
    files::append_to_last_line(temp_file);

    temp_file << entry << "\n";

    if (temp_file.fail()) {
        files::restore(file_path);
        return false;
    }

    files::accept_changes(file_path);
    return true;
}

static bool write_all(std::ofstream& file_stream, std::vector<entries::entry>& entries)
{
    const auto end = entries.crend();

    for (auto iterator = entries.crbegin(); iterator != end; iterator++) {
        const auto& entry = *iterator;

        if (!entry.null()) {
            file_stream << entry << "\n";

            if (file_stream.fail()) {
                return false;
            }
        }
    }

    return true;
}

bool entries::overwrite(uint which, const entries::entry& entry)
{
    if (!entry.null() && !entry.valid()) {
        return false;
    }

    const fs::path& file_path = entries_file_path();

    if (file_path.empty()) {
        return false;
    }

    const fs::path& temp_path = files::prepare_for_write(file_path);

    if (temp_path.empty()) {
        return false;
    }

    std::vector<entries::entry> entries = entries::read_all();

    if (which < 0 || which >= entries.size()) {
        return false;
    }

    if (entry.null()) {
        entries.erase(entries.begin() + which);
    } else {
        entries[which] = entry;
    }

    std::ofstream temp_file(temp_path.c_str());

    if (!write_all(temp_file, entries)) {
        files::restore(file_path);
        return false;
    }

    return files::accept_changes(file_path);
}

entries::entry entries::get(uint which)
{
    if (which == 0) {
        which = 1;
    }

    return entries::read_all(which).back();
}

bool entries::remove(uint which)
{
    return entries::overwrite(which, entries::entry());
}

std::vector<entries::entry> entries::read_all(uint limit)
{
    std::vector<entries::entry> entries(limit == 0 ? 3000 : limit);
    const fs::path& file_path = entries_file_path();

    if (file_path.empty()) {
        return entries;
    }

    std::ifstream entries_file(file_path.c_str());
    std::string line;

    // If we have to read all lines, we read them front-to-back
    // because files::get_previous_line() is actually very
    // slow due to heavy use of peek() and seekg().

    if (limit == 0) {
        while (std::getline(entries_file, line)) {
            entries::entry entry = parse(line);

            if (entry.valid()) {
                entries.push_back(entry);
            }
        }

        std::reverse(entries.begin(), entries.end());
    } else {
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
        } while (files::get_previous_line(entries_file, line) && (i < limit));
    }

    entries_file.close();

    return entries;
}
