#include "color.h"

std::ostream& cli::operator<<(std::ostream& stream, const cli::color& color)
{
    stream << static_cast<rang::fg>(color);

    return stream;
}
