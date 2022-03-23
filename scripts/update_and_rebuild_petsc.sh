#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

function exitIfExitCode() {
    # When --with-packages-download-dir=path is used, PETSc will print out how to
    # download external packages. A system code 10 is used in this case. We should
    # not consider it as error.
    if [ $1 -eq 10 ]; then
        exit 0
    elif [ $1 -ne 0 ]; then
        printf "There was an error. Exiting...\n"
        exit 1
    fi
}

PFX_STR=''
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

  # If users specify "--prefix", we need to make install
  if [[ "$i" == "--prefix="* ]]; then
    PFX_STR=$i
  fi
done

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo $SCRIPT_DIR

# Display help
if [[ -n "$help" ]]; then
  cd $SCRIPT_DIR/..
  echo "Usage: $0 [-h | --help | --fast | --skip-submodule-update | <PETSc options> ]"
  echo
  echo "-h | --help              Display this message and list of available PETSc options"
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
if [ -z "$PETSC_ARCH" ]; then
    export PETSC_ARCH=arch-moose
fi


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

# Set installation prefix if given
if [ ! -z "$PETSC_PREFIX" ]; then
  PFX_STR="--prefix=$PETSC_PREFIX"
fi

cd $SCRIPT_DIR/../petsc

# If we're not going fast, remove the build directory and reconfigure
if [ -z "$go_fast" ]; then
  rm -rf $SCRIPT_DIR/../petsc/$PETSC_ARCH

  source $SCRIPT_DIR/configure_petsc.sh
  configure_petsc "$PFX_STR" $*

  exitIfExitCode $?
fi

make all
exitIfExitCode $?

if [ ! -z "$PFX_STR" ]; then
  make install
  exitIfExitCode $?
fi

exit 0
