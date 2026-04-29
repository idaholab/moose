#!/bin/bash
set -euxo pipefail

if [ -z "${CONDUIT_GIT_REF:-}" ]; then
    echo "Build argument CONDUIT_GIT_REF not set"
    exit 1
fi
export CONDUIT_DIR=/opt/conduit
GIT_REMOTE="${GIT_REMOTE:?}"
BUILD_DIR="${BUILD_DIR:?}"
BUILD_JOBS="${BUILD_JOBS:?}"
MOOSE_ENV="${MOOSE_ENV:?}"

# Source environment
set +u
source "${MOOSE_ENV:?}"
set -u

# Clone
SRC_DIR="${BUILD_DIR}/conduit"
git clone --depth 1 "$GIT_REMOTE" "$SRC_DIR"
cd "$SRC_DIR"
git fetch --depth 1 origin "$CONDUIT_GIT_REF"
git checkout FETCH_HEAD
git submodule update --init --recursive

CONDUIT_SRC_DIR="$SRC_DIR" \
MOOSE_JOBS="$BUILD_JOBS" \
"${BUILD_DIR}/update_and_rebuild_conduit.sh"

# Cleanup
rm -rf "$SRC_DIR"

# Export to environment
echo "export CONDUIT_DIR=${CONDUIT_DIR}" >> "$MOOSE_ENV"
