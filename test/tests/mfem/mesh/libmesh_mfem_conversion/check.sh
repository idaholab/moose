#!/usr/bin/env bash
# Checks that two native MFEM mesh files are the same (barring some expected differences)

#Help
Help() {
    echo "Checks that data in two MFEM native mesh files are the same"
    echo ""
    echo "Usage: check.sh <file 1> <file 2> [--ignore-boundary]"
    echo ""
    echo "  --ignore-boundary  Do not compare the boundary elements of the meshes"
}
#Check correct number of args
if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    Help
    exit 1
fi
IGNORE_BOUNDARY=0
if [ "$#" -eq 3 ]; then
    if [ "$3" != "--ignore-boundary" ]; then
        Help
        exit 1
    fi
    IGNORE_BOUNDARY=1
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

# The name of the finite element collection holding the nodes of a higher-order mesh records
# where its control points are placed, which differs between meshes read by libMesh and by
# MFEM even when the node positions are the same, so it is not compared. The boundary
# elements are skipped when requested, for meshes whose boundaries are expected to differ.
# Use unique temporary files so that checks run concurrently do not overwrite each other.
TMP_LEFT=$(mktemp)
TMP_RIGHT=$(mktemp)
trap 'rm -f "${TMP_LEFT}" "${TMP_RIGHT}"' EXIT
FILTER='IGNORE_BOUNDARY==1 && /boundary/ {INSIDE=1; next}
    /vertices/ && INSIDE==1 {INSIDE=0; next};
    INSIDE!=1 && !/FiniteElementCollection/ {print $0}'
awk -v IGNORE_BOUNDARY="${IGNORE_BOUNDARY}" "${FILTER}" "${FILE_LEFT}" > "${TMP_LEFT}"
awk -v IGNORE_BOUNDARY="${IGNORE_BOUNDARY}" "${FILTER}" "${FILE_RIGHT}" > "${TMP_RIGHT}"

#Should be identical
#exit code will be 0 if no diff
git diff --no-index "${TMP_LEFT}" "${TMP_RIGHT}"
code=$?
if [ ! $code -eq 0 ]; then
    exit 1
fi
