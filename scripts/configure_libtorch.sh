#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

function get_variable()
{
  if [ -z "${!1}" ]; then
    >&2 echo "ERROR: Missing required variable $1"
  fi
  echo "${!1}"
}

function underlying_compiler() {
  local c="$1"

  if "$c" --showme:command >/dev/null 2>&1; then
    "$c" --showme:command
  elif "$c" -show >/dev/null 2>&1; then
    "$c" -show | awk '{print $1}'
  elif "$c" -compile-info >/dev/null 2>&1; then
    "$c" -compile-info | awk '{print $1}'
  else
    echo "$c"
  fi
}

# Configure libtorch with the default MOOSE configuration options
#
# Separated so that it can be used across all libtorch build scripts:
# - scripts/update_and_rebuild_libtorch.sh
#
# Arguments:
#   1. Path to the libtorch source directory
#   2. Path to the libtorch build directory
#   3. Path to the libtorch installation directory
#
# Remaining arguments will be appended to the cmake command verbatim
function configure_libtorch()
{
  ARGS=()
  if which ninja &> /dev/null; then
    ARGS+=("-GNinja")
  fi
  ARGS+=("${@:4}")
  # Documenting the flag choices:
  # -DCMAKE_INCLUDE_PATH and -DCMAKE_LIBRARY_PATH are needed to find the PETSc installation,
  #    which is needed to make sure BLAS and LAPACK are compatible with PETSc and vice versa.
  # -DBUILD_PYTHON=OFF, -DBUILD_TEST=OFF, -DBUILD_BINARY=OFF, and -DBUILD_LITE_INTERPRETER=OFF
  #    are needed to avoid building unnecessary targets that we don't need.
  # -DBUILD_FUNCTORCH=OFF, -DUSE_GLOO=OFF, -DUSE_NCCL=OFF, -DUSE_XCCL=OFF, and -DUSE_TENSORPIPE=OFF
  #    are needed to avoid building distributed training support, which we don't need.
  # -DUSE_MPI=OFF and -DUSE_DISTRIBUTED=OFF are needed to avoid building MPI support, which we don't need.
  # -DUSE_MAGMA=ON is needed to enable MAGMA support, which we want for GPU support on linear algebra operations on large matrices.
  # -DUSE_MKLDNN=OFF, -DUSE_NNPACK=OFF, -DUSE_XNNPACK=OFF, and -DUSE_PYTORCH_QNNPACK=OFF are
  #    needed to avoid building support for various CPU acceleration libraries that we don't need.
  # -DUSE_KINETO=OFF: we don't need Kineto because we don't use PyTorch's profiler
  # -DUSE_FBGEMM=OFF: we don't need FBGEMM because we don't use quantization
  # -DUSE_NUMPY=OFF: we don't need NumPy support because we don't use PyTorch's C++ API for Python bindings
  # -DUSE_ITT=OFF: we don't need ITT support because we don't use PyTorch's profiler
  # -DUSE_VALGRIND=OFF: we don't need Valgrind support because we don't use PyTorch's C++ API for Python bindings
  #    and we don't use PyTorch's profiler
  # -DUSE_OBSERVERS=OFF: we don't need observers support because we don't use PyTorch's profiler
  cmake \
    -DCMAKE_C_COMPILER="$(underlying_compiler "${CC:-cc}")" \
    -DCMAKE_CXX_COMPILER="$(underlying_compiler "${CXX:-c++}")" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$3" \
    -DBUILD_PYTHON=OFF \
    -DBUILD_TEST=OFF \
    -DBUILD_BINARY=OFF \
    -DBUILD_LITE_INTERPRETER=OFF \
    -DBUILD_FUNCTORCH=OFF \
    -DUSE_GLOO=OFF \
    -DUSE_NCCL=OFF \
    -DUSE_XCCL=OFF \
    -DUSE_TENSORPIPE=OFF \
    -DCMAKE_INCLUDE_PATH="$(get_variable PETSC_DIR)/include" \
    -DCMAKE_LIBRARY_PATH="$(get_variable PETSC_DIR)/lib" \
    -DUSE_BLAS=ON \
    -DBLAS=OpenBLAS \
    -DUSE_LAPACK=ON \
    -DUSE_MPI=OFF \
    -DUSE_DISTRIBUTED=OFF \
    -DUSE_MAGMA=ON \
    -DUSE_MKLDNN=OFF \
    -DUSE_NNPACK=OFF \
    -DUSE_XNNPACK=OFF \
    -DUSE_PYTORCH_QNNPACK=OFF \
    -DBUILD_TEST=OFF \
    -DUSE_KINETO=OFF \
    -DUSE_FBGEMM=OFF \
    -DUSE_NUMPY=OFF \
    -DUSE_ITT=OFF \
    -DUSE_VALGRIND=OFF \
    -DUSE_OBSERVERS=OFF \
    -B "$2" \
    -S "$1" \
    "${ARGS[@]}"
}

# Build libtorch assuming that the project has already been configured
#
# Arguments:
#   1. Path to the libtorch build directory
#   2. Number of jobs to use when building libtorch
function build_libtorch()
{
  cmake --build "$1" --parallel "$2" --target all
}

# Install libtorch
#
# Arguments:
#   1. Path to the libtorch build directory
#   2. Install prefix
function install_libtorch()
{
  cmake --install "$1" --prefix "$2"
}
