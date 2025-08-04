#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Configure libMesh with the default MOOSE configuration options
#
# Separated so that it can be used across all libMesh build scripts:
# - scripts/update_and_rebuild_libmesh.sh
# - conda/libmesh/build.sh
function configure_libmesh()
{
  if [ -z "$SRC_DIR" ]; then
    echo "SRC_DIR is not set for configure_libmesh"
    exit 1
  fi

  if [ ! -d "$SRC_DIR" ]; then
    echo "$SRC_DIR=SRC_DIR does not exist"
    exit 1
  fi

  if [ -z "$LIBMESH_DIR" ]; then
    echo "$LIBMESH_DIR is not set for configure_libmesh"
    exit 1
  fi

  # Preserves capability in update_and_rebuild_libmesh.sh, but this is set in
  # conda/libmesh/build.sh. If not, conda considers it an "unbound variable"
  if [[ -n "$INSTALL_BINARY" ]]; then
    echo "INFO: INSTALL_BINARY set"
  else
    export INSTALL_BINARY="${SRC_DIR}/build-aux/install-sh -C"
  fi

  # If METHODS is not set in update_and_rebuild_libmesh.sh, set a default value.
  export METHODS=${METHODS:="opt oprof devel dbg"}

  EXTRA_ARGS=()
  # libtirpc has changed paths from a previous default searched location
  if [[ $(uname) == Linux ]] && [[ -d ${CONDA_PREFIX}/include/tirpc ]]; then
    EXTRA_ARGS+=("--with-xdr-include=${CONDA_PREFIX}/include/tirpc")
  fi

  cd "${SRC_DIR}/build" || exit 1
  # shellcheck disable=SC2086  # we want wordsplitting
  # shellcheck disable=SC2048  # we want to not handle whitspaces
  ../configure --enable-silent-rules \
               --enable-unique-id \
               --disable-warnings \
               --with-thread-model=openmp \
               --disable-maintainer-mode \
               --enable-hdf5 \
               --enable-petsc-hypre-required \
               --enable-metaphysicl-required \
               --enable-xdr-required \
               --with-cxx-std-min=2014 \
               --without-gdb-command \
               --with-methods="${METHODS}" \
               --prefix="${LIBMESH_DIR}" \
               --with-future-timpi-dir="${LIBMESH_DIR}" \
               INSTALL="${INSTALL_BINARY}" \
               "${EXTRA_ARGS[@]}" \
               $*

  return $?
}
