#!/bin/bash
set -euxo pipefail

if [ -z "${MFEM_GIT_REF:-}" ]; then
    echo "Build argument MFEM_GIT_REF not set"
    exit 1
fi
export MFEM_DIR=/opt/mfem
GIT_REMOTE="${GIT_REMOTE:?}"
BUILD_DIR="${BUILD_DIR:?}"
BUILD_JOBS="${BUILD_JOBS:?}"
MOOSE_ENV="${MOOSE_ENV:?}"

# Source environment
set +u
source "${MOOSE_ENV:?}"
set -u

# Clone
SRC_DIR="${BUILD_DIR}/mfem"
git clone --depth 1 "$GIT_REMOTE" "$SRC_DIR"
cd "$SRC_DIR"
git fetch --depth 1 origin "$MFEM_GIT_REF"
git checkout FETCH_HEAD

# Additional options
MFEM_OPTIONS=()
# Cuda enabled
if [ -n "${CUDA_DIR:-}" ]; then
    MFEM_OPTIONS+=("-DMFEM_USE_CUDA=ON" "-DCUDA_ARCH=sm_80")
    # Expose the compat libraries
    export LD_LIBRARY_PATH=${CUDA_DIR}/compat:${LD_LIBRARY_PATH}
    # Expose the cuda headers
    export CPATH=${CUDA_DIR}/include:${CPATH}
fi

MFEM_SRC_DIR="$SRC_DIR" \
MOOSE_JOBS="$BUILD_JOBS" \
"${BUILD_DIR}/update_and_rebuild_mfem.sh"

# Cleanup
rm -rf "$SRC_DIR"

# Export to environment
echo "export MFEM_DIR=${MFEM_DIR}" >> "$MOOSE_ENV"
