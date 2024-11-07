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
  echo "Usage: $0 [-h | --help | -k | --version=VERSION_NUMBER | --libtorch-dest=DESTINATION | "
  echo " --libtorch-distribution=DISTRIBUTION | --cleanup ]"
  echo
  echo "-h | --help                          Display this message and list of available setup options"
  echo "-k                                   Ignore certifications while downloading packages with curl"
  echo "--cleanup                            Remove the downloaded tarball after the install"
  echo "--version=VERSION_NUMBER             Specify the version number of libtorch"
  echo "--libtorch-dest=DESTINATION          Specify where the packages are to be copied"
  echo "--libtorch-distribution=DISTRIBUTION Specify the distribution (cpu/cu118/cu121/cu124/rocm6.2)"
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
  VERSION=$VERSION.$REVISION
fi

# Little helper routine to check if a version number is lower
# or higher than a given number
version_check() {
  local loc_1=$1
  local loc_2=$2

  case $3 in
    -g);;
    -l)
      loc_1=$2
      loc_2=$1
      ;;
    *)
      echo "'version_check' function does only supports '-g' and '-l' for comparison!"
      exit 0
      ;;
  esac

  if { echo "$loc_1"; echo "$loc_2"; } | sort --version-sort --check=quiet; then
    false
  else
    true
  fi
}

# We check if somebody requested a very old version that we don't support
if version_check ${VERSION} 1.4.0 -l; then
  echo "ERROR! The current implementation does not support libtorch versions below 1.4!"
  exit 1
fi

# We clean up before we get a new version
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
if [[ $OP_SYS == mac ]]; then
  if version_check ${VERSION} 2.2.0 -l; then
    if [[ $UNAME_ARCH == "-arm64" ]]; then
      echo "ERROR! Precompiled libraries below version 2.2 are not available for ARM architecture!"
      exit 1
    else
      UNAME_ARCH=""
    fi
  fi
else
  UNAME_ARCH="" # We don't need this for linux machines
fi

# Helper routine for the glibc error message.
function error_message {
  echo "ERROR! The current version of GLIBC is not sufficient for proper linking!"
  echo "Upgrade it to at least $1! Current version: $2"
}

# We check if the available compiler stack is suitable for compiling moose with the
# precompiled versions of libtorch
if [[ $OP_SYS == linux ]]; then
  GLIBC_VERSION=`ldd --version | awk '/ldd/{print $NF}'`
  if !( version_check ${VERSION} 1.8.0 -g ); then
    if (( $(echo "$GLIBC_VERSION < 2.23" | bc -l) )); then
      error_message 2.23 $GLIBC_VERSION
      exit 1
    fi
  elif !( version_check ${VERSION} 2.1.0 -g ); then
    if (( $(echo "$GLIBC_VERSION < 2.27" | bc -l) )); then
      error_message 2.27 $GLIBC_VERSION
      exit 1
    fi
  else
    if (( $(echo "$GLIBC_VERSION < 2.29" | bc -l) )); then
      error_message 2.29 $GLIBC_VERSION
      exit 1
    fi
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

echo "******************************************************************************************
A precompiled libtorch (version: $VERSION distribution: $TORCH_DISTRIBUTION) has been
downloaded to $TORCH_DEST.
Next, you need to configure MOOSE to use the downloaded libtorch, i.e use the following
command in the root directory of moose:
  ./configure --with-libtorch
******************************************************************************************"

# Clean it up after if requested
if [[ $CLEANUP == 1 ]]; then
  echo "Removing $PACKAGE."
  rm $PACKAGE
fi
