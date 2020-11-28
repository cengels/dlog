#include <ctime>
#include "entry.h"

entries::entry::entry() : from(0), to(0)
{ }

bool entries::entry::valid() const
{
    time_t now = std::time(nullptr);

    return this->from > 0  // from must not be uninitialized
        && (this->to == 0 || this->to >= this->from)  // to must be uninitialized or greater than from
        && this->from <= now  // from must be smaller than now
        && this->to <= now;  // to must be smaller than now
}

bool entries::entry::complete() const
{
    return this->valid()
        && this->to > 0
        && !this->activity.empty();
}

bool entries::entry::null() const
{
    return this->from == 0
        && this->to == 0
        && this->activity.empty()
        && this->project.empty()
        && this->tags.empty()
        && this->comment.empty();
}

std::ostream& entries::operator<<(std::ostream& stream, const entries::entry& entry)
{
    if (!entry.valid()) {
        return stream;
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

    stream << "\",\""
           << entry.comment
           << "\"";

    return stream;
}
