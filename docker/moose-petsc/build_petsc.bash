#!/bin/bash
set -euxo pipefail

if [ -z "${PETSC_GIT_REF:-}" ]; then
    echo "Build argument PETSC_GIT_REF not set"
    exit 1
fi

# Clone PETSc
export PETSC_SRC_DIR="${BUILD_DIR:?}/petsc"
git clone --depth 1 "${PETSC_GIT_REMOTE:?}" "$PETSC_SRC_DIR"
cd "$PETSC_SRC_DIR"
git fetch --depth 1 origin "${PETSC_GIT_REF:?}"
git checkout FETCH_HEAD

# Source environment with MPI, possibly cuda, etc
set +u
source "${MOOSE_ENV:?}"
set -u

# Properly space options
IFS=' ' read -r -a PETSC_OPTIONS <<< "${PETSC_OPTIONS:-}"
# Flags needed for profiling
if [ -n "${PROFILING:-}" ]; then
    PETSC_OPTIONS+=("--CFLAGS=-fno-omit-frame-pointer" "--CXX_CXXFLAGS=-fno-omit-frame-pointer" "--FFLAGS=-fno-omit-frame-pointer")
fi
# Flags needed with cuda
if [ -n "${CUDA_DIR:-}" ]; then
    PETSC_OPTIONS+=("--with-cuda" "--with-cuda-dir=${CUDA_DIR}" "--with-cuda-arch=80" "--download-slate" "--download-hypre-configure-arguments='--enable-gpu-aware-mpi'")
    # Expose the compat libraries that can be linked to during configure for building without a GPU present
    export LD_LIBRARY_PATH="${CUDA_DIR}/compat:${LD_LIBRARY_PATH}"
fi

# Profit
cd "$BUILD_DIR"
PETSC_DIR=/opt/petsc
MOOSE_JOBS="${BUILD_JOBS:?}" PETSC_PREFIX="$PETSC_DIR" "${BUILD_DIR}/update_and_rebuild_petsc.sh" "${PETSC_OPTIONS[@]}"

# Test, only if not using cuda
if [ -z "${CUDA_DIR:-}" ]; then
    # Enable openmpi running as root for tests
    export OMPI_ALLOW_RUN_AS_ROOT=1
    export OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1

    cd "$PETSC_SRC_DIR"
    make SLEPC_DIR="$PETSC_DIR" PETSC_DIR="$PETSC_DIR" PETSC_ARCH= check
fi

# Remove example files and datafiles
rm -rf "${PETSC_DIR}/share/petsc/examples/src" "$PETSC_DIR/share/petsc/datafiles"

# Cleanup
rm -rf "$PETSC_SRC_DIR"

# Add directory to environment
echo "export PETSC_DIR=${PETSC_DIR}" >> "${MOOSE_ENV}"
