#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

cd "$(dirname "${BASH_SOURCE[0]}")"/..

WASP_SRC_DIR="`pwd`/wasp"

# Loop over specified command line arguments and turn on any found options

for ARG in "$@" ; do

  if [[ "${ARG}" == "--fast" ]] ; then FAST=1 ; fi

  if [[ "${ARG}" == "--help" ]] ; then HELP=1 ; fi

done

# Display the help menu to show the list of available options if requested

if [[ -n "${HELP}" ]] ; then

  echo "Usage: $0 [ --help | --fast ]"

  echo

  echo "--help    Display this message listing the options that are available"

  echo "--fast    Run WASP 'make install' only and do NOT update or configure"

  echo "*********************************************************************"

  echo

  exit 0

fi

# If we are going fast then check and make sure the build directory exists

if [[ -n "${FAST}"  ]] ; then

  if [[ ! -d "${WASP_SRC_DIR}"/build ]] ; then

    echo "Error: a build directory must exist to use the --fast option"

    exit 1

  fi

  cd "${WASP_SRC_DIR}"/build

# If we are not going fast then update WASP, remove build, and reconfigure

else

  git rev-parse 2> /dev/null

  if [[ $? -eq 0 ]] ; then

    git submodule update --init --recursive "${WASP_SRC_DIR}"

    if [[ $? -ne 0 ]] ; then

      echo "Error: git submodule update failed to complete successfully"

      mkdir -p "${WASP_SRC_DIR}"

      exit 1

    fi

  fi

  rm -rf "${WASP_SRC_DIR}"/build

  mkdir -p "${WASP_SRC_DIR}"/build

  cd "${WASP_SRC_DIR}"/build

  cmake \
    -D CMAKE_BUILD_TYPE:STRING=RELEASE \
    -D wasp_ENABLE_testframework:BOOL=OFF \
    -D wasp_ENABLE_TESTS:BOOL=OFF \
    -D CMAKE_INSTALL_PREFIX:STRING="${WASP_SRC_DIR}"/install \
    -D BUILD_SHARED_LIBS:BOOL=ON \
    -D DISABLE_HIT_TYPE_PROMOTION:BOOL=ON \
    "${WASP_SRC_DIR}"

  if [[ $? -ne 0 ]] ; then

    echo "Error: configure step for WASP failed to complete successfully"

    exit 1

  fi

fi

# Build with MOOSE_JOBS jobs if defined, otherwise build with a single job

make -j ${MOOSE_JOBS:-1} install

if [[ $? -ne 0 ]] ; then

  echo "Error: build step for WASP failed to complete successfully"

  exit 1

fi

exit 0
