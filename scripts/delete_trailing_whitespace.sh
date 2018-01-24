#!/bin/bash

# If a user supplies an argument, that directory will be used as the start point for finding files that contain whitespace, otherwise the MOOSE directory
# will be used (one up from the scripts directory where this script is located)
REPO_DIR=${1:-"$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../"}

echo "if you have clang-format installed, please use it instead of this script"

if [ ! -d "$REPO_DIR" ]; then
  echo "$REPO_DIR directory does not exist";
else
  while read -rd '' fname; do
    if grep -q "[[:blank:]]$" "$fname"; then
      echo "Removing trailing whitespace: $fname"
      perl -pli -e "s/\s+$//" "$fname" # this would also fix EOF issues
    elif [ "$(tail -c1 "$fname")" != "" ]; then
      echo "Adding newline at EOF: $fname"
      sed -i -e '$a\' "$fname"
    fi
  done < <( find "$REPO_DIR" -path ./contrib -prune -o -path ./libmesh -prune -o \( -name "*.[Chi]" -o -name "*.py" -o -name "*.tex" \) -type f -print0)
fi
