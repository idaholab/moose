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
  # Apple Silicon has an issue building shared libraries
  if [[ `uname -p` == "arm" ]]; then
    SHARED=0
  else
    SHARED=1
  fi

  cd $PETSC_DIR
  python ./configure --download-hypre=1 \
      --with-shared-libraries=$SHARED \
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
