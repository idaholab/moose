#!/bin/bash

# If a user supplies an argument, that directory will be used as the start point for finding files that contain whitespace, otherwise the MOOSE directory
# will be used (one up from the scripts directory where this script is located)
REPO_DIR=${1:-"$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../"}

if [ ! -d "$REPO_DIR" ]; then
  echo "$REPO_DIR directory does not exist";
else
  bad_files=$(find $REPO_DIR -path ./contrib -prune -o -path ./libmesh -prune -o \( -name "*.[Chi]" -o -name "*.py" \) -type f -print0 | xargs -0 grep -l "[[:blank:]]$")
  if [ "$bad_files" != "" ]; then
    # do it like this to split on new lines. This preserves files with spaces in their names.
    while read -r line; do
      echo "Fixing: $line"
      perl -pli -e "s/\s+$//" "$line"
    done <<< "$bad_files"
  fi
fi
