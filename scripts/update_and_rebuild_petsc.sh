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
  else # Remove the skip submodule update argument before passing to libMesh configure
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

# If --fast was used, it means we are going to skip configure, so
# don't allow the user to pass any other flags (with the exception
# of --skip-submodule-update) to the script thinking they are going
# to do something.
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
    # TODO: is this a git bug?
    # failed attempts with `git submodule update` deletes the submodule directory.
    # So re-create it to prevent a diff.
    mkdir libmesh
    exit 1
  fi
fi


cd $SCRIPT_DIR/../petsc

# If we're not going fast, remove the build directory and reconfigure
if [ -z "$go_fast" ]; then
  rm -rf $SCRIPT_DIR/../petsc/$PETSC_ARCH

  ./configure --with-debugging=no \
               --with-mpi=1 \
               --download-fblaslapack=1 \
               --download-metis=1 \
               --download-parmetis=1 \
               --download-superlu_dist=1 \
               --download-hypre=1 \
               --download-mumps=1 \
               --download-scalapack=1 \
               --download-ptscotch=1 \

   make all
else
   make all
fi
