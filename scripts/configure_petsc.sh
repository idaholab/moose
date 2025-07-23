#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Configure PETSc with the default MOOSE configuration options
#
# Separated so that it can be used across all PETSc build scripts:
# - scripts/update_and_rebuild_petsc.sh
# - scripts/update_and_rebuild_petsc_alt.sh
# - conda/petsc/build.sh
function configure_petsc()
{
  if [ -z "$PETSC_DIR" ]; then
    echo 'PETSC_DIR is not set for configure_petsc'
    exit 1
  fi
  if [ ! -d "$PETSC_DIR" ]; then
    echo "$PETSC_DIR=PETSC_DIR does not exist"
    exit 1
  fi

  # Whether or not we're building on apple silicon
  local IS_APPLE_SILICON=
  if [[ $(uname -p) == 'arm' ]] && [[ $(uname) == 'Darwin' ]] && [[ $PETSC_ARCH == 'arch-moose' ]]; then
    IS_APPLE_SILICON=1
  fi

  # Extra configure options to pass to petsc
  EXTRA_CONFIGURE_OPTIONS=()

  # Use --with-make-np if MOOSE_JOBS is given
  if [ -n "$MOOSE_JOBS" ]; then
    EXTRA_CONFIGURE_OPTIONS+=("--with-make-np=$MOOSE_JOBS")
  fi

  # Find the location for patches if needed
  local PATCH_DIR=
  if [ -n "$BASH_VERSION" ]; then
      PATCH_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
  elif [ -n "$ZSH_VERSION" ]; then
      PATCH_DIR="${0:A:h}"
  else
      echo "Not using Bash or Zsh. Script may not work as expected."
      exit 1
  fi

  # Check to see if HDF5 exists using environment variables and expected locations.
  # If it does, use it.
  echo 'INFO: Checking for HDF5...'

  # Prioritize user-set environment variables HDF5_DIR, HDF5DIR, and HDF5_ROOT,
  # with the first taking the greatest priority
  local FOUND_HDF5_DIR=""
  if [ -n "$HDF5_DIR" ]; then
    echo "INFO: HDF5 installation location was set using HDF5_DIR=$HDF5_DIR"
    FOUND_HDF5_DIR="$HDF5_DIR"
  elif [ -n "$HDF5DIR" ]; then
    echo "INFO: HDF5 installation location was set using HDF5DIR=$HDF5DIR"
    FOUND_HDF5_DIR="$HDF5DIR"
  elif [ -n "$HDF5_ROOT" ]; then
    echo "INFO: HDF5 installation location was set using HDF5_ROOT=$HDF5_ROOT"
    FOUND_HDF5_DIR="$HDF5_ROOT"
  fi
  # If not found using a variable, look at a few common library locations
  if [ -z "$FOUND_HDF5_DIR" ]; then
    local HDF5_PATHS=/usr/lib/hdf5:/usr/local/hdf5:/usr/share/hdf5:/usr/local/hdf5/share:/opt/hdf5:$HOME/.local
    # Set path delimiter
    IFS=:
    for p in $HDF5_PATHS; do
      # When first instance of hdf5 header is found, report finding, set HDF5_STR, and break
      loc=$(find "$p" -name 'hdf5.h' -print -quit 2>/dev/null)
      if [ -n "$loc" ]; then
        echo "INFO: HDF5 header location was found at: $loc"
        echo 'INFO: Using this HDF5 installation to configure and build PETSc.'
        echo 'INFO: If another HDF5 is desired, please set HDF5_DIR and re-run this script.'
        FOUND_HDF5_DIR="$loc/../../"
        break
      fi
    done
    unset IFS
  fi
  # Pass whatever we decided from HDF5_STR as an option
  if [ -n "$FOUND_HDF5_DIR" ]; then
    EXTRA_CONFIGURE_OPTIONS+=("--with-hdf5-dir=${FOUND_HDF5_DIR}")
  # Otherwise, download it via PETSc and patch if on apple silicon
  else
    EXTRA_CONFIGURE_OPTIONS+=("--download-hdf5=1" "--with-hdf5-fortran-bindings=0" "--download-hdf5-configure-arguments='--with-zlib'")
    echo 'INFO: HDF5 library not detected, opting to download via PETSc...'
    if [ -n "$IS_APPLE_SILICON" ]; then
      echo 'INFO: Patching PETSc to support HDF5 download and installation on Apple Silicon...'
      local HDF5_PATCH=$PATCH_DIR/apple-silicon-hdf5-autogen.patch
      git apply "$HDF5_PATCH" 2>/dev/null || (git apply "$HDF5_PATCH" -R --check && echo 'INFO: Apple Silicon HDF5 patch already applied.')
    fi
  fi

  # When manually building PETSc on Apple Silicon, set FFLAGS to the proper arch, otherwise MUMPS
  # will fail to find MPI libraries
  if [ -n "$IS_APPLE_SILICON" ]; then
    EXTRA_CONFIGURE_OPTIONS+=("FFLAGS=-march=armv8.3-a")
  fi

  cd "$PETSC_DIR" || exit 1
  python3 ./configure --with-64-bit-indices \
      --with-cxx-dialect=C++17 \
      --with-debugging=no \
      --with-fortran-bindings=0 \
      --with-mpi=1 \
      --with-openmp=1 \
      --with-strict-petscerrorcode=1 \
      --with-shared-libraries=1 \
      --with-sowing=0 \
      --download-fblaslapack=1 \
      --download-hpddm=1 \
      --download-hypre=1 \
      --download-metis=1 \
      --download-mumps=1 \
      --download-ptscotch=1 \
      --download-parmetis=1 \
      --download-scalapack=1 \
      --download-slepc=1 \
      --download-strumpack=1 \
      --download-superlu_dist=1 \
      --download-kokkos=1 \
      --download-kokkos-kernels=1 \
      --download-libceed=1 \
      "${EXTRA_CONFIGURE_OPTIONS[@]}" \
      "$@"

  local RETURN_CODE=$?
  if [ $RETURN_CODE != 0 ] && [ -f configure.log ]; then
    echo "Configure failed; displaying contents of configure.log:"
    cat configure.log
  fi

  return $RETURN_CODE
}
