#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $SCRIPT_DIR/..
find . \( -name *.[Chi] -or -name *.py -or \( -name "contrib" -or -name "libmesh" \) -prune -and -type f \) -print0 | xargs -0 perl -pli -e "s/\s+$//"
