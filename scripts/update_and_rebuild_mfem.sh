#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
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
    skip_sub_update=1;
  else # Remove everything else before passing to cmake
    set -- "$@" "$i"
  fi
done

# Display help
if [ -n "$help" ]; then
  echo "Usage: $0 [-h | --help | --fast | --skip-submodule-update | <mfem cmake options> ]"
  echo
  echo "-h | --help              Display this message and list of available libmesh options"
  echo "--skip-submodule-update  Do not update the libMesh submodule, use the current version"
  echo
  echo "Influential variables"
  echo "CONDUIT_SRC_DIR          Path to conduit; default: ../framework/contrib/conduit from submodule"
  echo "LIBMESH_DIR              Path to libmesh (for netcdf); default ../libmesh/installed"
  echo "MFEM_DIR                 MFEM install prefix; default: ../framework/contrib/mfem/installed"
  echo "MFEM_SRC_DIR             Path to MFEM source; default: ../framework/contrib/mfem from submodule"
  echo "PETSC_ARCH               PETSc arch; default: arch-moose if PETSC_DIR not set"
  echo "PETSC_DIR                Path to PETSc install; default: ../petsc"
  exit 0
fi

set -e

if [ -n "$MFEM_SRC_DIR" ]; then
  skip_sub_update=1
else
  MFEM_SRC_DIR="$(realpath -m "${SCRIPT_DIR}"/../framework/contrib/mfem)"
fi
MFEM_BUILD_DIR="${MFEM_SRC_DIR}/build"
if [ -n "$MFEM_DIR" ]; then
  echo "INFO: MFEM_DIR set - overriding default installed path"
  echo "INFO: No cleaning will be done in specified path"
else
  MFEM_DIR="${MFEM_SRC_DIR}/installed"
  rm -rf "$MFEM_DIR"
fi

CONDUIT_BUILD_DIR="${CONDUIT_SRC_DIR}/build"
LIBMESH_DIR=${LIBMESH_DIR:-$(realpath -m "${SCRIPT_DIR}/../libmesh/installed")}
if [ -z "$PETSC_DIR" ]; then
  PETSC_DIR=$(realpath -m "${SCRIPT_DIR}/../petsc/arch-moose")
fi

if [ -z "$skip_sub_update" ]; then
  cd "${SCRIPT_DIR}/.."
  git submodule update --init --checkout framework/contrib/conduit
  git submodule update --init --checkout framework/contrib/mfem
  cd framework/contrib/conduit
  git submodule update --init
fi

# Clean up old builds
rm -rf "$CONDUIT_BUILD_DIR" "$MFEM_BUILD_DIR"

# Build and install conduit
mkdir -p "$CONDUIT_BUILD_DIR"
cd "$CONDUIT_BUILD_DIR"
cmake ../src -DCMAKE_INSTALL_PREFIX="$MFEM_DIR"
make -j "${MOOSE_JOBS:-4}"
make install

# Build and install mfem
mkdir -p "$MFEM_BUILD_DIR"
cd "$MFEM_BUILD_DIR"
cmake .. \
    -DCMAKE_POSITION_INDEPENDENT_CODE=YES \
    -DMFEM_USE_OPENMP=NO \
    -DMFEM_THREAD_SAFE=NO \
    -DHYPRE_DIR="$PETSC_DIR" \
    -DPETSC_DIR="$PETSC_DIR" \
    -DMFEM_USE_MPI=YES \
    -DMFEM_USE_METIS=YES \
    -DMFEM_USE_METIS_5=YES \
    -DMETIS_DIR="$PETSC_DIR" \
    -DParMETIS_DIR="$PETSC_DIR" \
    -DMFEM_USE_SUPERLU=YES \
    -DSuperLUDist_DIR="$PETSC_DIR" \
    -DBUILD_SHARED_LIBS=ON \
    -DHDF5_DIR="$PETSC_DIR" \
    -DBLAS_DIR="$PETSC_DIR" \
    -DMFEM_ENABLE_EXAMPLES=yes \
    -DMFEM_ENABLE_MINIAPPS=yes \
    -DBLAS_LIBRARIES="$PETSC_DIR/lib/libfblas.a" \
    -DLAPACK_LIBRARIES="$PETSC_DIR/lib/libflapack.a" \
    -DBLAS_INCLUDE_DIRS="$PETSC_DIR/include" \
    -DLAPACK_INCLUDE_DIRS="$PETSC_DIR/include" \
    -DCMAKE_INSTALL_PREFIX="$MFEM_DIR" \
    -DMFEM_USE_PETSC=YES \
    -DPETSC_DIR="$PETSC_DIR" \
    -DPETSC_ARCH="$PETSC_ARCH" \
    -DCMAKE_C_COMPILER=mpicc \
    -DMFEM_USE_NETCDF=YES \
    -DNETCDF_DIR="$LIBMESH_DIR" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DMFEM_USE_CONDUIT=YES \
    -DCONDUIT_DIR="$MFEM_DIR" \
    "$@"
make -j "${MOOSE_JOBS:-4}"
make install
