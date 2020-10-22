#include <experimental/filesystem>
#include <fstream>
#include "files.h"
#include "entries.h"

entries::entry::entry() : from(0), to(0)
{ }

bool entries::entry::valid() const
{
    return !this->activity.empty()
        && this->from != 0;
}

bool entries::entry::complete() const
{
    return this->valid()
        && this->to != 0;
}

std::string entries::serialize(const entries::entry& entry)
{
    if (!entry.valid()) {
        return std::string();
    }

    std::string result;

    result.append(std::to_string(entry.from));
    result.append(",");
    result.append(std::to_string(entry.to));
    result.append(",\"");
    result.append(entry.activity);
    result.append("\",\"");
    result.append(entry.project);
    result.append("\",\"");

    const auto& last = entry.tags.back();

    for (const std::string& tag : entry.tags) {
        result.append(tag);

        if (tag != last) {
            result.append(",");
        }
    }

    result.append("\"");

    return result;
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
    std::experimental::filesystem::path entries_file_path = files::dlog_file("entries");

    if (entries_file_path.empty()) {
        return entries::entry();
    }

    std::ifstream entries_file(entries_file_path.c_str());
    const std::string last_line = files::get_last_line(entries_file);
    entries_file.close();

    return parse(last_line);
}
