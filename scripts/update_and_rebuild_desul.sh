#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

for i in "$@"
do
  shift
  if [[ "$i" == "-h" || "$i" == "--help" ]]; then
    help=1;
  fi

  if [ "$i" == "--skip-submodule-update" ]; then
    skip_sub_update=1
  elif [[ "$i" == "--with-cuda" || "$i" == "--with-hip" || "$i" == "--with-sycl" ]]; then
    if [ -n "$backend" ]; then
      echo "Error: only one of --with-cuda, --with-hip, and --with-sycl may be given"
      exit 1
    fi
    backend="${i#--with-}"
  else # Remove everything else before passing to cmake
    set -- "$@" "$i"
  fi
done

# Display help
if [ -n "$help" ]; then
  echo "Usage: $0 [-h | --help | --skip-submodule-update | --with-cuda | --with-hip | --with-sycl | <desul cmake options> ]"
  echo
  echo "-h | --help              Display this message"
  echo "--skip-submodule-update  Do not update the desul submodule, use the current version"
  echo "--with-cuda              Enable CUDA atomics; must match the PETSc GPU backend"
  echo "--with-hip               Enable HIP atomics; must match the PETSc GPU backend"
  echo "--with-sycl              Enable SYCL atomics; must match the PETSc GPU backend"
  echo
  echo "Influential variables"
  echo "DESUL_SRC_DIR            Path to desul; default: ../framework/contrib/desul from submodule"
  echo "DESUL_DIR                desul install prefix; default: ../framework/contrib/desul/installed"
  echo "CUDA_COMPILER            CUDA compiler used with --with-cuda; default: nvcc"
  echo "HIP_COMPILER             HIP compiler used with --with-hip; default: hipcc"
  echo "SYCL_COMPILER            SYCL compiler used with --with-sycl; default: icpx"
  exit 0
fi

set -e

if [ -n "$DESUL_SRC_DIR" ]; then
  skip_sub_update=1
else
  DESUL_SRC_DIR="$(realpath "${SCRIPT_DIR}"/../framework/contrib/desul/.)"
fi
DESUL_BUILD_DIR="${DESUL_SRC_DIR}/build"
if [ -n "$DESUL_DIR" ]; then
  echo "INFO: DESUL_DIR set - overriding default installed path"
  echo "INFO: No cleaning will be done in specified path"
else
  DESUL_DIR="${DESUL_SRC_DIR}/installed"
  rm -rf "$DESUL_DIR"
fi

if [ -z "$skip_sub_update" ]; then
  cd "${SCRIPT_DIR}/.."
  git submodule update --init --checkout framework/contrib/desul
  # desul configures through BLT, which is its own submodule
  cd framework/contrib/desul
  git submodule update --init
fi

# Clean up old builds
rm -rf "$DESUL_BUILD_DIR"

ARGS=()
# Use ninja for build if available
if which ninja &> /dev/null; then
  ARGS+=("-GNinja")
fi

# Enable the GPU atomics backend through the BLT options; compiler defaults match
# framework/kokkos.mk
case "$backend" in
  cuda)
    ARGS+=("-DENABLE_CUDA=ON" "-DCMAKE_CUDA_COMPILER=${CUDA_COMPILER:-nvcc}") ;;
  hip)
    ARGS+=("-DENABLE_HIP=ON" "-DCMAKE_CXX_COMPILER=${HIP_COMPILER:-hipcc}") ;;
  sycl)
    ARGS+=("-DENABLE_SYCL=ON" "-DCMAKE_CXX_COMPILER=${SYCL_COMPILER:-icpx}") ;;
esac

# Build and install desul
mkdir -p "$DESUL_BUILD_DIR"
cd "$DESUL_BUILD_DIR"
cmake .. \
  -DCMAKE_INSTALL_PREFIX="$DESUL_DIR" \
  -DBUILD_SHARED_LIBS=ON \
  -DENABLE_TESTS=OFF \
  -DDESUL_ENABLE_OPENMP=OFF \
  "${ARGS[@]}" \
  "$@"
cmake --build . --parallel "${MOOSE_JOBS:-4}"
cmake --install .
