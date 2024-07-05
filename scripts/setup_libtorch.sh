#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# For now, 2.1.0 is the default version of libtorch
VERSION=2.1.0
IGNORE_CERT=""

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TORCH_DEST=$SCRIPT_DIR/../framework/contrib
TORCH_DISTRIBUTION="cpu"

# Parse the input arguments
for i in "$@"
do
  shift
  if [[ "$i" == "--cleanup" ]]; then
    CLEANUP=1
  elif [[ "$i" == "--version="* ]]; then
    VSTR="$i"
    VERSION=${VSTR#*=};
  elif [[ "$i" == "--libtorch-dest="* ]]; then
    VSTR="$i"
    TORCH_DEST=${VSTR#*=};
  elif [[ "$i" == "--libtorch-distribution="* ]]; then
    VSTR="$i"
    TORCH_DISTRIBUTION=${VSTR#*=};
  elif [[ "$i" == "-h" || "$i" == "--help" ]]; then
    HELP=1;
  elif [[ "$i" == "-k" ]]; then
    IGNORE_CERT="-k";
  fi
done

# Show how to use the script
if [[ -n "$HELP" ]]; then
  echo "Usage: $0 [-h | --help | -k | --version=VERSION_NUMBER | --cleanup ]"
  echo
  echo "-h | --help              Display this message and list of available setup options"
  echo "-k                       Ignore certifications while downloading packages with curl"
  echo "--cleanup                Remove the downloaded tarball after the install"
  echo "--version=VERSION_NUMBER Specify the version number of libtorch"
  echo "*************************************************************************************"
  echo ""
  exit 0
fi

# Create a directory path
TORCH_DIR=$TORCH_DEST/libtorch

# We need to do this because 1.12 is lower than 1.9 if parsed as a single number
MAINVERSION=$(echo $VERSION | cut -d. -f1)
SUBVERSION=$(echo $VERSION | cut -d. -f2)
REVISION=$(echo $VERSION | cut -d. -f3)

if [[ -z "${REVISION}" ]]; then
  REVISION="0"
  VERSION=$VERSION.0
fi

if (( $MAINVERSION < 1  || ( $MAINVERSION == 1  &&  $SUBVERSION < 4 ) )); then
  echo "The current implementation does not support libtorch versions below 1.4!"
  exit 1
fi

if [ -d $TORCH_DIR ]; then
  echo "Cleaning up previous libtorch installation"
  rm -rf $TORCH_DIR
fi

# Checking the operating system and architecture type
UNAME_SYS="$(uname -s)"
UNAME_ARCH="-$(uname -m)"
case "${UNAME_SYS}" in
  Linux*)     OP_SYS=linux;;
  Darwin*)    OP_SYS=mac;;
esac

# We do some sanity checking on the version number. They started distributing
# precompiled libraries for ARM machines afer version 2.2. Before that, it was x86 without the
# tag
if (( $OP_SYS == mac )); then
  if (( $MAINVERSION < 2  || ( $MAINVERSION == 2 && $SUBVERSION < 2 ) )); then
    if (( UNAME_ARCH == "-arm64" )); then
      echo "Precompiled libraries below version 2.2 are not available for ARM architecture!"
      exit 1
    else
      UNAME_ARCH=""
    fi
  fi
else
  UNAME_ARCH="" # We don't need this for linux machines
fi

# Checking if the available GLIBC version is sufficient for proper linkig. Only
# causes issues on linux distributions. Considering that most Macs use the
# moose compiler stack.
function version_greater {
  if { echo "$1"; echo "$2"; } | sort --version-sort --check=quiet; then
    false
  else
    true
  fi
}
function error_message {
  echo "ERROR! The current version of GLIBC is not sufficient for proper linking!"
  echo "Upgrade it to at least $1! Current version: $2"
}

if [[ $OP_SYS == linux ]]; then
  GLIBC_VERSION=`ldd --version | awk '/ldd/{print $NF}'`
  if not version_greater $VERSION 1.8 && (( $(echo "$GLIBC_VERSION < 2.23" | bc -l) )); then
    error_message 2.23 $GLIBC_VERSION
    exit 1
  elif not version_greater $VERSION 2.1 && (( $(echo "$GLIBC_VERSION < 2.27" | bc -l) )); then
    error_message 2.27 $GLIBC_VERSION
    exit 1
  elif version_greater $VERSION 2.1 && (( $(echo "$GLIBC_VERSION < 2.29" | bc -l) )); then
    error_message 2.29 $GLIBC_VERSION
    exit 1
  fi
fi

# Depending on the distribution, download the corresponding precompiled libs
# Also, if another installation is present for libtorch, we overwrite the files.
FILENAME=""
if [[ $OP_SYS == linux ]]; then
  FILENAME=libtorch-cxx11-abi-shared-with-deps-$VERSION%2B$TORCH_DISTRIBUTION.zip
elif [[ $OP_SYS == mac ]]; then
  FILENAME=libtorch-macos$UNAME_ARCH-$VERSION.zip
else
  echo "Unknown operating system! We only support Linux/Mac machines!"
  exit 1
fi

# Check if it is necessary to download a new package or we already have one
PACKAGE=${TORCH_DEST}/${FILENAME}
if [[ -f $PACKAGE ]]; then
  echo "Found requested package for libtorch v. $VERSION, no need to download."
else
  curl -L $IGNORE_CERT -o $PACKAGE https://download.pytorch.org/libtorch/$TORCH_DISTRIBUTION/$FILENAME
fi
echo "Extracting $PACKAGE."
unzip -q -o $PACKAGE -d $TORCH_DEST

# Clean it up after if requested
if [[ $CLEANUP == 1 ]]; then
  echo "Removing $PACKAGE."
  rm $PACKAGE
fi
