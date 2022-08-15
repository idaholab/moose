#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
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
    echo "PETSC_DIR is not set for configure_petsc"
    exit 1
  fi
  if [ ! -d "$PETSC_DIR" ]; then
    echo "$PETSC_DIR=PETSC_DIR does not exist"
    exit 1
  fi

  # Use --with-make-np if MOOSE_JOBS is given
  MAKE_NP_STR=""
  if [ ! -z "$MOOSE_JOBS" ]; then
    MAKE_NP_STR="--with-make-np=$MOOSE_JOBS"
  fi

  # Check to see if HDF5 exists using environment variables and expected locations.
  # If it does, use it.
  echo "INFO: Checking for HDF5..."

  # Prioritize user-set environment variables HDF5_DIR, HDF5DIR, and HDF5_ROOT,
  # with the first taking the greatest priority
  if [ -n "$HDF5_DIR" ]; then
    echo "INFO: HDF5 installation location was set using HDF5_DIR=$HDF5_DIR"
    HDF5_STR="--with-hdf5-dir=$HDF5_DIR"
  elif [ -n "$HDF5DIR" ]; then
    echo "INFO: HDF5 installation location was set using HDF5DIR=$HDF5DIR"
    HDF5_STR="--with-hdf5-dir=$HDF5DIR"
  elif [ -n "$HDF5_ROOT" ]; then
    echo "INFO: HDF5 installation location was set using HDF5_ROOT=$HDF5_ROOT"
    HDF5_STR="--with-hdf5-dir=$HDF5_ROOT"
  fi

  # If not found using a variable, look at a few common library locations
  HDF5_PATHS=/usr/lib/hdf5:/usr/local/hdf5:/usr/share/hdf5:/usr/local/hdf5/share:$HOME/.local
  if [ -z "$HDF5_STR" ]; then
    # Set path delimiter
    IFS=:
    for p in $HDF5_PATHS; do
      # When first instance of hdf5 header is found, report finding, set HDF5_STR, and break
      loc=$(find "$p" -name 'hdf5.h' -print -quit 2>/dev/null)
      if [ ! -z "$loc" ]; then
        echo "INFO: HDF5 header location was found at: $loc"
        echo "INFO: Using this HDF5 installation to configure and build PETSc."
        echo "INFO: If another HDF5 is desired, please set HDF5_DIR and re-run this script."
        HDF5_STR="--with-hdf5-dir=$loc/../../"
        break
      fi
    done
    unset IFS
  fi

  # If HDF5 is not found locally, download it via PETSc
  HDF5_FORTRAN_STR=""
  if [ -z "$HDF5_STR" ]; then
    HDF5_STR="--download-hdf5=1"
    HDF5_FORTRAN_STR="--download-hdf5-fortran-bindings=0"
    echo "INFO: HDF5 library not detected, opting to download via PETSc..."
  fi

  # If manually building PETSc on Apple Silicon (arm64), several adjustments need to be made to
  # properly configure PETSc for this platform
  MUMPS_ARM_STR=""
  if [[ `uname -p` == "arm" ]] && [[ $(uname) == Darwin ]] && [[ $PETSC_ARCH == arch-moose ]]; then
    echo "INFO: Apple Silicon detected, checking to see if PETSc ARM patches need to be applied..."
    # First check to see if patch marker file exists in PETSC_DIR due to a previous build. If not,
    # perform patch and create patch marker file. If so, skip patch and report.
    patch_file=$(find "$PETSC_DIR" -name '.patched' -print -quit 2>/dev/null)
    if [ -z "$patch_file" ]; then
      # If an HDF5 download is requested, patch PETSc to properly configure it
      if [ "$HDF5_STR" == "--download-hdf5=1" ]; then
         echo "INFO: Patching PETSc to support HDF5 download and installation on ARM..."
         git apply $PETSC_DIR/../scripts/apple-silicon-hdf5-autogen.patch
         touch $PETSC_DIR/.patched
      else
        echo "INFO: No ARM patches required, proceeding to PETSc configure..."
      fi
    elif [ ! -z "$patch_file" ]; then
      echo "INFO: Applicable ARM patches already applied, proceeding to PETSc configure..."
    fi
    # Finally, be sure to set FFLAGS for mumps to use the proper arch, otherwise it will fail to
    # build correctly
    MUMPS_ARM_STR="FFLAGS="-march=armv8.3-a""
  fi

  cd $PETSC_DIR
  python ./configure --download-hypre=1 \
      --with-shared-libraries=1 \
      "$HDF5_STR" \
      "$HDF5_FORTRAN_STR" \
      "$MAKE_NP_STR" \
      "$MUMPS_ARM_STR" \
      --with-debugging=no \
      --download-fblaslapack=1 \
      --download-metis=1 \
      --download-ptscotch=1 \
      --download-parmetis=1 \
      --download-superlu_dist=1 \
      --download-mumps=1 \
      --download-strumpack=1 \
      --download-scalapack=1 \
      --download-slepc=1 \
      --with-mpi=1 \
      --with-openmp=1 \
      --with-cxx-dialect=C++11 \
      --with-fortran-bindings=0 \
      --with-sowing=0 \
      --with-64-bit-indices \
      "$@"

  return $?
}
