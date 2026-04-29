#!/bin/bash
set -euxo pipefail

if [ -z "${LIBTORCH_VERSION:-}" ]; then
    echo "Build argument LIBTORCH_VERSION not set"
    exit 1
fi
LIBTORCH_DIR=/opt/libtorch
BUILD_DIR="${BUILD_DIR:?}"
BUILD_JOBS="${BUILD_JOBS:?}"

# Source environment
set +u
source "${MOOSE_ENV:?}"
set -u

# Figure out compiler and version which will affect flags
COMPILER=$(mpicxx -show | awk '{ print $1 }')
# Explicitly silence all warnings because the build is loud (>5MB of output...)
BUILD_CCXXFLAGS=("-w")
BUILD_FLAGS=()
if [ "$COMPILER" == "g++" ]; then
    BUILD_CCXXFLAGS+=("-Wno-error=maybe-uninitialized" "-Wno-error=uninitialized" "-Wno-error=restrict")
elif [ "$COMPILER" == "clang++" ]; then
    # FBGEMM requires AVX512, which we don't get with clang
    BUILD_FLAGS+=("-DUSE_FBGEMM:BOOL=OFF")
else
    echo "Unknown compiler $COMPILER"
    exit 1
fi


# Clone for build
cd "$BUILD_DIR"
git clone --branch "v${LIBTORCH_VERSION}" --single-branch --recursive https://github.com/pytorch/pytorch
SRC_DIR="${BUILD_DIR}/pytorch"

# Create a virtual python environment for building pytorch
VENV_DIR="${BUILD_DIR}/venv"
python -m venv "$VENV_DIR"
source "${VENV_DIR}/bin/activate"
which python
python --version
pip3 install --no-cache -r "${SRC_DIR}/requirements.txt"

# Require CUDA if we have cuda available
if [ -n "${CUDA_DIR:-}" ]; then
    # 7.0 V100 (sawtooth), 8.0 A100 (hoodoo), 8.6 A2 (milangpubuild), 8.9 L4 (rod, cone)
    export CUDA_ARCH_LIST="8.0;8.6;8.9"
    BUILD_FLAGS+=("-DUSE_CUDA:BOOL=ON" "-DCUDA_TOOLKIT_ROOT_DIR=${CUDA_DIR}")
else
    BUILD_FLAGS+=("-DUSE_CUDA:BOOL=OFF")
fi

# Build and install
LIBTORCH_BUILD_DIR=${BUILD_DIR}/pytorch_build
mkdir ${LIBTORCH_BUILD_DIR} && cd ${LIBTORCH_BUILD_DIR}
CC= CXX= F77= F90= FC= CFLAGS="${BUILD_CCXXFLAGS[*]}" CXXFLAGS="${BUILD_CCXXFLAGS[*]}" cmake \
    -GNinja \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DCMAKE_BUILD_TYPE:STRING=Release \
    -DCMAKE_INSTALL_PREFIX:PATH="$LIBTORCH_DIR" \
    -DBUILD_PYTHON:BOOL=OFF \
    -DCMAKE_INCLUDE_PATH:PATH="${PETSC_DIR}/include" \
    -DCMAKE_LIBRARY_PATH:PATH="${PETSC_DIR}/lib" \
    -DUSE_BLAS:BOOL=ON \
    -DBLAS:STRING=OpenBLAS \
    -DUSE_LAPACK:BOOL=ON \
    -DUSE_MPI:BOOL=OFF \
    -DUSE_DISTRIBUTED:BOOL=OFF \
    -DUSE_NNPACK:BOOL=OFF \
    -DUSE_XNNPACK:BOOL=OFF \
    -DUSE_PYTORCH_QNNPACK:BOOL=OFF \
    -DBUILD_TEST:BOOL=OFF \
    -DUSE_KINETO:BOOL=OFF \
    -DUSE_FBGEMM:BOOL=OFF \
    "${BUILD_FLAGS[@]}" \
    "$SRC_DIR"
cmake --build . --target install -- -j "$BUILD_JOBS"

# Cleanup
deactivate
rm -rf "$LIBTORCH_BUILD_DIR" "$SRC_DIR" "$VENV_DIR"

# Expose libtorch
echo "export LIBTORCH_DIR=${LIBTORCH_DIR}" >> "$MOOSE_ENV"
