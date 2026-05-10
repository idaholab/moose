#!/bin/bash
set -euxo pipefail

if [ -z "${NEML2_GIT_REF:-}" ]; then
    echo "Build argument NEML2_GIT_REF not set"
    exit 1
fi
export NEML2_DIR=/opt/neml2
GIT_REMOTE="${GIT_REMOTE:?}"
BUILD_DIR="${BUILD_DIR:?}"
BUILD_JOBS="${BUILD_JOBS:?}"
MOOSE_ENV="${MOOSE_ENV:?}"

# Source environment
set +u
source "${MOOSE_ENV:?}"
set -u

# Clone
SRC_DIR="${BUILD_DIR}/neml2"
git clone --depth 1 "$GIT_REMOTE" "$SRC_DIR"
cd "$SRC_DIR"
git fetch --depth 1 origin "$NEML2_GIT_REF"
git checkout FETCH_HEAD
git submodule update --init --recursive

METHODS="opt devel dbg" \
NEML2_SRC_DIR="$SRC_DIR" \
MOOSE_JOBS="$BUILD_JOBS" \
"${BUILD_DIR}/update_and_rebuild_neml2.sh" --skip-submodule-update

# Cleanup
rm -rf "$SRC_DIR"

# Export to environment
echo "export NEML2_DIR=${NEML2_DIR}" >> "$MOOSE_ENV"
