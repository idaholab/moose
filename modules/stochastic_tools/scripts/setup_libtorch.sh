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
VERSION=1.10

# Parse the input arguments
for i in "$@"
do
  shift
  if [[ "$i" == "--version="* ]]; then
    VSTR="$i"
    VERSION=${VSTR#*=};
  elif [[ "$i" == "-h" || "$i" == "--help" ]]; then
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

MAINVERSION=${VERSION%%.*}
SUBVERSION=${VERSION##*.}

if (( $SUBVERSION < 4 || $MAINVERSION < 1 )); then
  echo "The current implementation does not support libtorch versions below 1.4!"
  exit 1
fi

# Checking the operating system
UNAME_OUT="$(uname -s)"
case "${UNAME_OUT}" in
  Linux*)     OP_SYS=linux;;
  Darwin*)    OP_SYS=mac;;
esac

# Checking if the available GLIBC version is sufficient for proper linkig. Only
# causes issues on llinux distributions. Considering that most Macs use the
# moose compiler stack.
if [[ $OP_SYS == linux ]]; then
  GLIBC_VERSION=`ldd --version | awk '/ldd/{print $NF}'`
  if (( $SUBVERSION < 8 && $(echo "$GLIBC_VERSION < 2.23" | bc -l) )); then
    echo "The current version of GLIBC is not sufficient for proper linking!"
    echo "Upgrade it to at least 2.23! Current version: $GLIBC_VERSION"
    exit 1
  elif (( $SUBVERSION > 8 && $(echo "$GLIBC_VERSION < 2.27" | bc -l) )); then
    echo "The current version of GLIBC is not sufficient for proper linking!"
    echo "Upgrade it to at least 2.27! Current version: $GLIBC_VERSION"
    exit 1
  fi
fi

# Depending on the distribution, download the corresponding precompiled libs
# Also, if another installation is present for libtorch, we overwrite the files.
if [[ $OP_SYS == linux ]]; then
  if [[ -f "libtorch-cxx11-abi-shared-with-deps-$VERSION.0%2Bcpu.zip" ]]; then
    echo "Found requested package for libtorch v. $VERSION, no need to download."
  else
    curl -L -O https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-$VERSION.0%2Bcpu.zip
  fi
  echo "Extracting libtorch-cxx11-abi-shared-with-deps-$VERSION.0%2Bcpu.zip."
  unzip -q -o libtorch-cxx11-abi-shared-with-deps-$VERSION.0%2Bcpu.zip
elif [[ $OP_SYS == mac ]]; then
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
