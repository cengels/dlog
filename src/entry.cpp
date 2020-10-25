#include "entry.h"

entries::entry::entry() : from(0), to(0)
{ }

bool entries::entry::valid() const
{
    return this->from > 0
        && (this->to == 0 || this->to >= this->from);
}

bool entries::entry::complete() const
{
    return this->valid()
        && this->to > 0
        && !this->activity.empty();
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

    stream << "\"";

    return stream;
}
