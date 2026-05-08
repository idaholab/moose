#!/usr/bin/env bash
Help() {
    echo "Checks that data in two csv files are the same"
    echo ""
    echo "Usage: check_csv.sh <file 1> <file 2>"
}
# Check correct number of args
if [ ! "$#" -eq 2 ]; then
    Help
    exit 1
fi
# Check files exist
FILE_LEFT=$1
FILE_RIGHT=$2
if [ ! -f "${FILE_LEFT}" ]; then
    echo "${FILE_LEFT} doesn't exist"
    exit 1
fi
if [ ! -f "${FILE_RIGHT}" ]; then
    echo "${FILE_RIGHT} doesn't exist"
    exit 1
fi
# Filter out the header and diff the rest
# Should be identical
# exit code will be 0 if no diff
diff <(tail -n +2 $FILE_LEFT) <(tail -n +2 $FILE_RIGHT) > /dev/null
code=$?
if [ ! $code -eq 0 ]; then
    exit 1
fi
