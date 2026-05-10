#!/bin/bash
set -euxo pipefail

if [ -z "${WASP_GIT_REF:-}" ]; then
    echo "Build argument WASP_GIT_REF not set"
    exit 1
fi
export WASP_DIR=/opt/wasp
GIT_REMOTE="${GIT_REMOTE:?}"
BUILD_DIR="${BUILD_DIR:?}"
BUILD_JOBS="${BUILD_JOBS:?}"
MOOSE_ENV="${MOOSE_ENV:?}"

# Source environment
set +u
source "${MOOSE_ENV:?}"
set -u

# Clone
SRC_DIR="${BUILD_DIR}/wasp"
git clone --depth 1 "$GIT_REMOTE" "$SRC_DIR"
cd "$SRC_DIR"
git fetch --depth 1 origin "$WASP_GIT_REF"
git checkout FETCH_HEAD
git submodule update --init --recursive

WASP_SRC_DIR="$SRC_DIR" \
MOOSE_JOBS="$BUILD_JOBS" \
"${BUILD_DIR}/update_and_rebuild_wasp.sh" -D CMAKE_INSTALL_PREFIX:STRING="$WASP_DIR"

# Cleanup
rm -rf "$SRC_DIR"

# Export to environment
echo "export WASP_DIR=${WASP_DIR}" >> "$MOOSE_ENV"
