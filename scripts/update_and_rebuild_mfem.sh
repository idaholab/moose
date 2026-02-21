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

  if [ "$i" == "--fast" ]; then
    go_fast=1;
  fi

  if [ "$i" == "--skip-submodule-update" ]; then
    skip_sub_update=1
  else # Remove everything else before passing to cmake
    set -- "$@" "$i"
  fi
done

# Display help
if [ -n "$help" ]; then
  echo "Usage: $0 [-h | --help | --fast | --skip-submodule-update | <mfem cmake options> ]"
  echo
  echo "-h | --help              Display this message and list of available libmesh options"
  echo "--fast                   Run MFEM 'make' only, do NOT run CMake"
  echo "--skip-submodule-update  Do not update the MFEM submodule, use the current version"
  echo
  echo "Influential variables"
  echo "CONDUIT_DIR              Path to conduit; default: ../framework/contrib/conduit/installed"
  echo "LIBMESH_DIR              Path to libmesh (for netcdf); default: ../libmesh/installed"
  echo "MFEM_DIR                 MFEM install prefix; default: ../framework/contrib/mfem/installed"
  echo "MFEM_SRC_DIR             Path to MFEM source; default: ../framework/contrib/mfem from submodule"
  echo "PETSC_ARCH               PETSc arch; default: arch-moose if PETSC_DIR not set"
  echo "PETSC_DIR                Path to PETSc install; default: ../petsc"
  echo "HDF5_DIR                 Path to HDF5 install; default: \$PETSC_DIR/\$PETSC_ARCH"
  echo "GSLIB_DIR                Path to GSLIB source git repo"
  exit 0
fi

if [[ -n "$go_fast" && $# != 1 ]]; then
  echo "Error: --fast can only be used by itself or with --skip-submodule-update."
  echo "Try again, removing either --fast or all other conflicting arguments!"
  exit 1;
fi

set -e

if [ -n "$MFEM_SRC_DIR" ]; then
  skip_sub_update=1
else
  MFEM_SRC_DIR=$(realpath "${SCRIPT_DIR}/../framework/contrib/mfem/.")
fi
MFEM_BUILD_DIR_BASE="${MFEM_SRC_DIR}/build"
if [ -n "$MFEM_DIR" ]; then
  echo "INFO: MFEM_DIR set - overriding default installed path"
  echo "INFO: No cleaning will be done in specified path"
else
  MFEM_DIR="${MFEM_SRC_DIR}/installed"
  rm -rf "$MFEM_DIR"
fi

: ${CONDUIT_DIR:=$(realpath "${SCRIPT_DIR}/../framework/contrib/conduit/installed/.")}
: ${LIBMESH_DIR:=$(realpath "${SCRIPT_DIR}/../libmesh/installed/.")}
if [ -z "$PETSC_DIR" ]; then
  PETSC_DIR=$(realpath "${SCRIPT_DIR}/../petsc/.")
  PETSC_ARCH="arch-moose"
fi
: ${HDF5_DIR:=$PETSC_DIR/$PETSC_ARCH}

# Overwrite GSLIB repo URL if GSLIB_DIR looks like a git repo
if [ -n "$GSLIB_DIR" ] && [ -d "$GSLIB_DIR/.git" ]; then
  export GIT_CONFIG_COUNT=1
  export GIT_CONFIG_KEY_0=url.file://$GSLIB_DIR.insteadOf
  export GIT_CONFIG_VALUE_0=https://github.com/Nek5000/gslib
fi

if [ -z "$skip_sub_update" ]; then
  git submodule update --init --checkout "${MFEM_SRC_DIR}"
fi

# Set of supported build methods
SUPPORTED_METHODS="oprof devel dbg opt"

# If METHODS is not set by the user, set it to METHOD if set by the user,
# otherwise default to building all supported methods listed above
: ${METHODS:=${METHOD:-$SUPPORTED_METHODS}}

# Map from the supported libMesh-like methods to CMake's build types
build_type_pairs=(
  "oprof=PROFILE"
  "devel=RELWITHDEBINFO"
  "dbg=DEBUG"
  "opt=RELEASE"
)

get_build_type() {
  local key="$1"
  local pair
  for pair in "${build_type_pairs[@]}"; do
    case "$pair" in
      "$key="*) echo "${pair#*=}"; return 0 ;;
    esac
  done
  return 1
}

# The order here, i.e. in METHODS, _is_ important: the libraries for the last
# of the requested methods will be available for external use (see below)
for METHOD in $METHODS
do
  [[ $SUPPORTED_METHODS =~ $METHOD ]] ||
  { echo "Error: Build method $METHOD is not recognised, choose from $SUPPORTED_METHODS."; exit 1; }


  CMAKE_BUILD_TYPE="$(get_build_type "$METHOD")"

  # If we're not going fast, remove the build directory and reconfigure
  if [ -z "$go_fast" ]; then
    rm -rf "$MFEM_BUILD_DIR_BASE-$METHOD"
    mkdir -p "$MFEM_BUILD_DIR_BASE-$METHOD"
    cd "$MFEM_BUILD_DIR_BASE-$METHOD"

    cmake .. \
      -DBUILD_SHARED_LIBS=YES \
      -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
      -DCMAKE_${CMAKE_BUILD_TYPE}_POSTFIX=-$METHOD \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
      -DCMAKE_INSTALL_PREFIX="$MFEM_DIR" \
      -DCMAKE_C_FLAGS_PROFILE="-O2 -g -DNDEBUG -fno-omit-frame-pointer" \
      -DCMAKE_CXX_FLAGS_PROFILE="-O2 -g -DNDEBUG -fno-omit-frame-pointer" \
      -DCMAKE_CUDA_FLAGS_PROFILE="-O2 -g -DNDEBUG -fno-omit-frame-pointer" \
      \
      -DMFEM_USE_CEED=YES \
      -DMFEM_USE_CONDUIT=YES \
      -DMFEM_USE_GSLIB=YES \
      -DMFEM_USE_MPI=YES \
      -DMFEM_USE_MUMPS=YES \
      -DMFEM_USE_NETCDF=YES \
      -DMFEM_USE_PETSC=YES \
      -DMFEM_USE_SUPERLU=YES \
      \
      -DMFEM_FETCH_GSLIB=YES \
      \
      -DCEED_DIR="$PETSC_DIR/$PETSC_ARCH" \
      -DCONDUIT_DIR="$CONDUIT_DIR" \
      -DHDF5_DIR="$HDF5_DIR" \
      -DHYPRE_DIR="$PETSC_DIR/$PETSC_ARCH" \
      -DMETIS_DIR="$PETSC_DIR/$PETSC_ARCH" \
      -DMUMPS_DIR="$PETSC_DIR/$PETSC_ARCH" \
      -DNETCDF_DIR="$LIBMESH_DIR" \
      -DParMETIS_DIR="$PETSC_DIR/$PETSC_ARCH" \
      -DPETSC_ARCH="$PETSC_ARCH" \
      -DPETSC_DIR="$PETSC_DIR" \
      -DScaLAPACK_ROOT="$PETSC_DIR/$PETSC_ARCH" \
      -DSuperLUDist_DIR="$PETSC_DIR/$PETSC_ARCH" \
      \
      -DBLAS_LIBRARIES="$PETSC_DIR/$PETSC_ARCH/lib/libfblas.a" \
      -DLAPACK_LIBRARIES="$PETSC_DIR/$PETSC_ARCH/lib/libflapack.a" \
      \
      "$@"
  fi

  cd "$MFEM_BUILD_DIR_BASE-$METHOD" ||
  { echo "Error: Need to run this script without --fast at least once."; exit 1; }

  make -j ${MOOSE_JOBS:-4} install
  cd miniapps/common
  make -j ${MOOSE_JOBS:-4} install

  # Save the configuration file for this build method
  mv "$MFEM_DIR/include/mfem/config/_config.hpp" "$MFEM_DIR/include/mfem/config/_config-$METHOD.hpp"

  # These symlinks, though unused by MOOSE apps, guarantee the installed config
  # file works for one, the last one, of the MFEM builds, enabling external use
  ln -sf _config-$METHOD.hpp "$MFEM_DIR/include/mfem/config/_config.hpp"
  for lib in "$MFEM_DIR"/lib/libmfem*-$METHOD*; do ln -sf $(basename "$lib") "${lib/-$METHOD/}"; done
done
