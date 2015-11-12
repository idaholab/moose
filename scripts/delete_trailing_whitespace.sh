#!/bin/bash

# If a user supplies an argument, that directory will be used as the start point for finding files that contain whitespace, otherwise the MOOSE directory
# will be used (one up from the scripts directory where this script is located)
REPO_DIR=${1:-"$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../"}

if [ ! -d "$REPO_DIR" ]; then
  echo "$REPO_DIR directory does not exist";
else
  find $REPO_DIR -path ./contrib -prune -o -path ./libmesh -prune -o \( -name "*.[Chi]" -o -name "*.py" \) -type f -print0 | xargs -0 perl -pli -e "s/\s+$//"
fi
