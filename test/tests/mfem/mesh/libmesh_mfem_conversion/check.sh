#!/usr/bin/env bash
# Checks that two native MFEM mesh files are the same (barring some expected differences)

#Help
Help() {
    echo "Checks that data in two MFEM native mesh files are the same"
    echo ""
    echo "Usage: check.sh <file 1> <file 2>"
}
#Check correct number of args
if [ ! "$#" -eq 2 ]; then
    Help
    exit 1
fi
#Check files exist
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

# MFEM doesn't seem to reorder boundary elements consistently, so filter them out.
# It also assumes a different spacing of control points in higher-order meshes
# than does libMesh.
awk '/boundary/ {INSIDE=1; next}
    /vertices/ && INSIDE==1 {INSIDE=0; next};
    INSIDE!=1 && !/FiniteElementCollection/ {print $0}' "${FILE_LEFT}" > tmp_left
awk '/boundary/ {INSIDE=1; next}
    /vertices/ && INSIDE==1 {INSIDE=0; next};
    INSIDE!=1 && !/FiniteElementCollection/ {print $0}' "${FILE_RIGHT}" > tmp_right

#Should be identical
#exit code will be 0 if no diff
git diff --no-index tmp_left tmp_right
code=$?
rm tmp_left tmp_right
if [ ! $code -eq 0 ]; then
    exit 1
fi
