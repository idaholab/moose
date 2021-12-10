#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# For now, 1.10 is the default version of torchlib
VERSION="1.10"

# Parse the input arguments
for i in "$@"
do
  shift
  if [[ "$i" == "--version="* ]]; then
    VSTR="$i"
    VERSION=${VSTR#*=};
  fi

  if [[ "$i" == "-h" || "$i" == "--help" ]]; then
    HELP=1;
  fi
done

# Show how to use the script
if [[ -n "$HELP" ]]; then
  echo "Usage: $0 [-h | --help | --version=VERSION_NUMBER ]"
  echo
  echo "-h | --help              Display this message and list of available setup options"
  echo "--version=VERSION_NUMBER Specify the version number of libtorch"
  echo "*************************************************************************************"
  echo ""
  exit 0
fi

# Checking the operating system
UNAME_OUT="$(uname -s)"
case "${UNAME_OUT}" in
  linux*)     OP_SYS=linux;;
  darwin*)    OP_SYS=mac;;
esac

# Depending on the distribution, download the corresponding precompiled libs
# Also, if another installation is present for libtorch, we overwrite the files.
if [[ $OP_SYS -eq linux ]]; then
  if [[ -f "libtorch-cxx11-abi-shared-with-deps-$VERSION.0%2Bcpu.zip" ]]; then
    echo "Found requested package for libtorch v. $VERSION, no need to download."
  else
    curl -L -O https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-$VERSION.0%2Bcpu.zip
  fi
  echo "Extracting libtorch-cxx11-abi-shared-with-deps-$VERSION.0%2Bcpu.zip."
  unzip -q -o libtorch-cxx11-abi-shared-with-deps-$VERSION.0%2Bcpu.zip
elif [[ $OP_SYS -eq mac ]]; then
  if [[ -f libtorch-macos-$VERSION.0.zip ]]; then
    echo "Found requested package for libtorch v. $VERSION, no need to download."
  else
    curl -L -O https://download.pytorch.org/libtorch/cpu/libtorch-macos-$VERSION.0.zip
  fi
  echo "Extracting libtorch-macos-$VERSION.0.zip."
  unzip -q -o libtorch-macos-$VERSION.0.zip
else
  echo "Unknown operating system! We only support Linux/Mac machines!"
  exit 1
fi
