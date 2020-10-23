#include "config.h"
#include "format.h"

std::string format::as_local_time_string(const time_t& time)
{
    char time_string[30];

    strftime(time_string, sizeof(time_string), config::config().time_format.c_str(), localtime(&time));

    return std::string(time_string);
}
