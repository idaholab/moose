#!/usr/bin/env bash

# Set go_fast flag if "--fast" is found in command line args.
for i in "$@"
do
  shift
  if [ "$i" == "--fast" ]; then
    go_fast=1;
  fi

  if [[ "$i" == "-h" || "$i" == "--help" ]]; then
    help=1;
  fi

  if [ "$i" == "--skip-submodule-update" ]; then
    skip_sub_update=1;
  else # Remove the skip submodule update argument before passing to PETSc configure
    set -- "$@" "$i"
  fi
done

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo $SCRIPT_DIR

# Display help
if [[ -n "$help" ]]; then
  cd $SCRIPT_DIR/..
  echo "Usage: $0 [-h | --help | --fast | --skip-submodule-update | <libmesh options> ]"
  echo
  echo "-h | --help              Display this message and list of available libmesh options"
  echo "--fast                   Run PETSc 'make all' only, do NOT run configure"
  echo "--skip-submodule-update  Do not update the PETSc submodule, use the current version"
  echo "*************************************************************************************"
  echo ""

  if [ -e "./petsc/configure" ]; then
    cd petsc
    ./configure -h
  fi
  exit 0
fi


if [[ -n "$go_fast" && $# != 1 ]]; then
  echo "Error: --fast can only be used by itself or with --skip-submodule-update."
  echo "Try again, removing either --fast or all other conflicting arguments!"
  exit 1;
fi

# Set PETSc envir
export PETSC_DIR=$SCRIPT_DIR/../petsc
export PETSC_ARCH=arch-moose


cd $SCRIPT_DIR/..

# Test for git repository when not using fast
git_dir=`git rev-parse --show-cdup 2>/dev/null`
if [[ -z "$go_fast" && -z "$skip_sub_update" && $? == 0 && "x$git_dir" == "x" ]]; then
  git submodule update --init --recursive petsc
  if [[ $? != 0 ]]; then
    echo "git submodule command failed, are your proxy settings correct?"
    exit 1
  fi
fi


cd $SCRIPT_DIR/../petsc

# If we're not going fast, remove the build directory and reconfigure
if [ -z "$go_fast" ]; then
  rm -rf $SCRIPT_DIR/../petsc/$PETSC_ARCH

  ./configure --download-hypre=1 \
      --with-ssl=0 \
      --with-debugging=no \
      --with-pic=1 \
      --with-shared-libraries=1 \
      --with-cc=mpicc \
      --with-cxx=mpicxx \
      --with-fc=mpif90 \
      --download-fblaslapack=1 \
      --download-metis=1 \
      --download-ptscotch=1 \
      --download-parmetis=1 \
      --download-superlu_dist=1 \
      --download-mumps=1 \
      --download-scalapack=1 \
      -CC=mpicc -CXX=mpicxx -FC=mpif90 -F77=mpif77 -F90=mpif90 \
      -CFLAGS='-fPIC -fopenmp' \
      -CXXFLAGS='-fPIC -fopenmp' \
      -FFLAGS='-fPIC -fopenmp' \
      -FCFLAGS='-fPIC -fopenmp' \
      -F90FLAGS='-fPIC -fopenmp' \
      -F77FLAGS='-fPIC -fopenmp' \

   make all
else
   make all
fi
