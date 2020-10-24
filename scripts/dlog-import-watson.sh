#!/bin/bash

# This program imports time entries (frames) from Watson CLI to dlog.
# It accepts two arguments: the source file and the target file.

if [[ $# -lt 2 ]] ; then
    echo 'Not enough arguments. Must supply at least source file name and target file name.'
    exit 1
fi

# watson stores its frames as JSON arrays in the format
# [from_time, to_time, project, frame_id, tags, updated_at]
# so we use jq to parse each array, turn the tags into a string
# and then convert each array into a CSV line.

jq -r 'map([.[0], .[1], .[2], "", (.[4] | join(","))] | @csv) | join("\n")' "$1" > "$2"

count=`jq length "$1"`

echo -e "Imported "$count" frames from watson.
To convert a watson project and tags to activity:project, \
you could run a search and replace across "$2"."
