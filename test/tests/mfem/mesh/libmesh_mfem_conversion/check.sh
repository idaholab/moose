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
# Use unique temporary files so that checks run concurrently do not overwrite each other.
TMP_LEFT=$(mktemp)
TMP_RIGHT=$(mktemp)
trap 'rm -f "${TMP_LEFT}" "${TMP_RIGHT}"' EXIT
awk '/boundary/ {INSIDE=1; next}
    /vertices/ && INSIDE==1 {INSIDE=0; next};
    INSIDE!=1 && !/FiniteElementCollection/ {print $0}' "${FILE_LEFT}" > "${TMP_LEFT}"
awk '/boundary/ {INSIDE=1; next}
    /vertices/ && INSIDE==1 {INSIDE=0; next};
    INSIDE!=1 && !/FiniteElementCollection/ {print $0}' "${FILE_RIGHT}" > "${TMP_RIGHT}"

#Should be identical
#exit code will be 0 if no diff
git diff --no-index "${TMP_LEFT}" "${TMP_RIGHT}"
code=$?
if [ ! $code -eq 0 ]; then
    exit 1
fi
