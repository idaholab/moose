#!/bin/bash
set -euxo pipefail

if [ -z "${LIBMESH_GIT_REF:-}" ]; then
    echo "Build argument LIBMESH_GIT_REF not set"
    exit 1
fi
export LIBMESH_DIR=/opt/libmesh
METHODS="${METHODS:?}"
GIT_REMOTE="${GIT_REMOTE:?}"
BUILD_DIR="${BUILD_DIR:?}"
BUILD_JOBS="${BUILD_JOBS:?}"
MOOSE_ENV="${MOOSE_ENV:?}"

# Clone libMesh
SRC_DIR="${BUILD_DIR}/libmesh"
git clone --depth 1 "$GIT_REMOTE" "$SRC_DIR"
cd "$SRC_DIR"
git fetch --depth 1 origin "$LIBMESH_GIT_REF"
git checkout FETCH_HEAD
git submodule update --init --recursive

# Source environment with MPI, possibly cuda, etc
set +u
source "$MOOSE_ENV"
set -u

# Properly space options
IFS=' ' read -r -a LIBMESH_OPTIONS <<< "${LIBMESH_OPTIONS:-}"
# Flags needed with cuda
if [ -n "${CUDA_DIR:-}" ]; then
    LIBMESH_OPTIONS+=("--with-nvtx=${CUDA_DIR}" "--enable-perflog")
fi

# Profit
METHODS="$METHODS" LIBMESH_SRC_DIR="$SRC_DIR" "${BUILD_DIR}/update_and_rebuild_libmesh.sh" "${LIBMESH_OPTIONS[@]}" --skip-submodule-update
rm -rf "$SRC_DIR"

# Add directory to environment
echo "export LIBMESH_DIR=${LIBMESH_DIR}" >> "$MOOSE_ENV"
