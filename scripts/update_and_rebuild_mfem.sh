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
  echo "LIBMESH_DIR              Path to libmesh (for netcdf); default ../libmesh/installed"
  echo "MFEM_DIR                 MFEM install prefix; default: ../framework/contrib/mfem/installed"
  echo "MFEM_SRC_DIR             Path to MFEM source; default: ../framework/contrib/mfem from submodule"
  echo "PETSC_ARCH               PETSc arch; default: arch-moose if PETSC_DIR not set"
  echo "PETSC_DIR                Path to PETSc install; default: ../petsc"
  echo "HDF5_DIR                 Path to HDF5 install; default: \$PETSC_DIR/\$PETSC_ARCH"
  exit 0
fi

if [[ -n "$go_fast" && $# != 1 ]]; then
  echo "Error: --fast can only be used by itself or with --skip-submodule-update."
  echo "Try again, removing either --fast or all other conflicting arguments!"
  exit 1;
fi

set -e

get_realpath() {
    python3 -c "import os, sys; print(os.path.realpath(sys.argv[1]))" "$1"
}

if [ -n "$MFEM_SRC_DIR" ]; then
  skip_sub_update=1
else
  MFEM_SRC_DIR="$(get_realpath "${SCRIPT_DIR}"/../framework/contrib/mfem)"
fi
MFEM_BUILD_DIR_BASE="${MFEM_SRC_DIR}/build"
if [ -n "$MFEM_DIR" ]; then
  echo "INFO: MFEM_DIR set - overriding default installed path"
  echo "INFO: No cleaning will be done in specified path"
else
  MFEM_DIR="${MFEM_SRC_DIR}/installed"
  rm -rf "$MFEM_DIR"
fi

if [ -n "$CONDUIT_SRC_DIR" ]; then
  skip_conduit_update=1
else
  CONDUIT_SRC_DIR="$(get_realpath "${SCRIPT_DIR}"/../framework/contrib/conduit)"
fi
CONDUIT_BUILD_DIR="${CONDUIT_SRC_DIR}/build"

CONDUIT_DIR=${CONDUIT_DIR:-$(get_realpath "${SCRIPT_DIR}/../framework/contrib/conduit/installed")}
LIBMESH_DIR=${LIBMESH_DIR:-$(get_realpath "${SCRIPT_DIR}/../libmesh/installed")}
if [ -z "$PETSC_DIR" ]; then
  PETSC_DIR=$(get_realpath "${SCRIPT_DIR}/../petsc")
  PETSC_ARCH="arch-moose"
fi
HDF5_DIR=${HDF5_DIR:-$PETSC_DIR/$PETSC_ARCH}

if [ -z "$skip_sub_update" ]; then
  cd "${SCRIPT_DIR}/.."
  git submodule update --init --checkout framework/contrib/mfem
fi

# Set of supported build methods
SUPPORTED_METHODS="devel dbg opt"

# If METHODS is not set by the user, set it to METHOD if set by the user,
# otherwise default to building all supported methods listed above
: ${METHODS:=${METHOD:-$SUPPORTED_METHODS}}

# Map from the supported libMesh-like methods to CMake's build types
typeset -A METHOD_TO_CMAKE_BUILD_TYPE_MAP
METHOD_TO_CMAKE_BUILD_TYPE_MAP=([devel]=RELWITHDEBINFO [dbg]=DEBUG [opt]=RELEASE)

# The order here, i.e. in METHODS, _is_ important: the libraries for the last
# of the requested methods will be available for external use (see below)
for METHOD in $METHODS
do
  [[ $SUPPORTED_METHODS =~ $METHOD ]] ||
  { echo "Error: Build method $METHOD is not recognised, choose from $SUPPORTED_METHODS."; exit 1; }

  CMAKE_BUILD_TYPE=${METHOD_TO_CMAKE_BUILD_TYPE_MAP[$METHOD]}

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
      \
      -DMFEM_ENABLE_MINIAPPS=YES \
      \
      -DMFEM_USE_CEED=YES \
      -DMFEM_USE_CONDUIT=YES \
      -DMFEM_USE_MPI=YES \
      -DMFEM_USE_NETCDF=YES \
      -DMFEM_USE_PETSC=YES \
      -DMFEM_USE_SUPERLU=YES \
      \
      -DCEED_DIR="$PETSC_DIR/$PETSC_ARCH" \
      -DCONDUIT_DIR="$CONDUIT_DIR" \
      -DHDF5_DIR="$HDF5_DIR" \
      -DHYPRE_DIR="$PETSC_DIR/$PETSC_ARCH" \
      -DMETIS_DIR="$PETSC_DIR/$PETSC_ARCH" \
      -DNETCDF_DIR="$LIBMESH_DIR" \
      -DParMETIS_DIR="$PETSC_DIR/$PETSC_ARCH" \
      -DPETSC_ARCH="$PETSC_ARCH" \
      -DPETSC_DIR="$PETSC_DIR" \
      -DSuperLUDist_DIR="$PETSC_DIR/$PETSC_ARCH" \
      \
      -DBLAS_LIBRARIES="$PETSC_DIR/$PETSC_ARCH/lib/libfblas.a" \
      -DLAPACK_LIBRARIES="$PETSC_DIR/$PETSC_ARCH/lib/libflapack.a" \
      "$@"
  fi

  cd "$MFEM_BUILD_DIR_BASE-$METHOD" ||
  { echo "Error: Need to run this script without --fast at least once."; exit 1; }

  make -j ${MOOSE_JOBS:-4}
  make -j ${MOOSE_JOBS:-4} install

  # Save the configuration file for this build method
  mv "$MFEM_DIR/include/mfem/config/_config.hpp" "$MFEM_DIR/include/mfem/config/_config-$METHOD.hpp"

  # These symlinks, though unused by MOOSE apps, guarantee the installed config
  # file works for one, the last one, of the MFEM builds, enabling external use
  ln -sf _config-$METHOD.hpp "$MFEM_DIR/include/mfem/config/_config.hpp"
  for lib in "$MFEM_DIR"/lib/libmfem*-$METHOD*; do ln -sf $(basename "$lib") "${lib/-$METHOD/}"; done
done
