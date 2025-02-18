#!/usr/bin/env bash
# Checks that the data in field with Name = <arg1> in in file <arg2>
# is the same as the data in field Nmae = <arg3> in file <arg4>
#Help
Help() {
    echo "Checks that data in two .vtu (xml) files are the same"
    echo ""
    echo "Usage: check.sh <field 1> <file 1> <field 2> <file 2>"
}
#Check correct number of args
if [ ! "$#" -eq 4 ]; then
    Help
    exit 1
fi
#Check files exist
FILE_LEFT=$2
FILE_RIGHT=$4
if [ ! -f "${FILE_LEFT}" ]; then
    echo "${FILE_LEFT} doesn't exist"
    exit 1
fi
if [ ! -f "${FILE_RIGHT}" ]; then
    echo "${FILE_RIGHT} doesn't exist"
    exit 1
fi
#Could use xmlstartlet or some other tool but not always installed
#hence awk filter out the values from the data fields
FIELD_LEFT=${1}
FIELD_RIGHT=${3}
awk -v var="${FIELD_LEFT}" \
    '/<DataArray/ && $0 ~ var {INSIDE=1; next}
    /<\/DataArray/ && INSIDE==1 {INSIDE=0; next};
    INSIDE==1 {print $0}' "${FILE_LEFT}" > tmp_left

awk -v var="${FIELD_RIGHT}" \
    '/<DataArray/ && $0 ~ var {INSIDE=1; next};
    /<\/DataArray>/ && INSIDE==1 {INSIDE=0; next};
    INSIDE==1 {print $0}' "${FILE_RIGHT}" > tmp_right
#Should be identical
#exit code will be 0 if no diff
diff tmp_left tmp_right > /dev/null
code=$?
rm tmp_left tmp_right
if [ ! $code -eq 0 ]; then
    exit 1
fi
