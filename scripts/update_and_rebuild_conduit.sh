#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

for i in "$@"
do
  shift
  if [[ "$i" == "-h" || "$i" == "--help" ]]; then
    help=1;
  fi

  if [ "$i" == "--skip-submodule-update" ]; then
    skip_sub_update=1
  else # Remove everything else before passing to cmake
    set -- "$@" "$i"
  fi
done

# Display help
if [ -n "$help" ]; then
  echo "Usage: $0 [-h | --help | --fast | --skip-submodule-update | <conduit cmake options> ]"
  echo
  echo "-h | --help              Display this message and list of available libmesh options"
  echo "--skip-submodule-update  Do not update the conduit submodule, use the current version"
  echo
  echo "Influential variables"
  echo "CONDUIT_SRC_DIR          Path to conduit; default: ../framework/contrib/conduit from submodule"
  echo "CONDUIT_DIR              conduit install prefix; default: ../framework/contrib/conduit/installed"
  exit 0
fi

set -e

if [ -n "$CONDUIT_SRC_DIR" ]; then
  skip_sub_update=1
else
  CONDUIT_SRC_DIR="$(realpath -m "${SCRIPT_DIR}"/../framework/contrib/conduit)"
fi
CONDUIT_BUILD_DIR="${CONDUIT_SRC_DIR}/build"
if [ -n "$CONDUIT_DIR" ]; then
  echo "INFO: CONDUIT_DIR set - overriding default installed path"
  echo "INFO: No cleaning will be done in specified path"
else
  CONDUIT_DIR="${CONDUIT_SRC_DIR}/installed"
  rm -rf "$CONDUIT_DIR"
fi

if [ -z "$skip_sub_update" ]; then
  cd "${SCRIPT_DIR}/.."
  git submodule update --init --checkout framework/contrib/conduit
  cd framework/contrib/conduit
  git submodule update --init
fi

# Clean up old builds
rm -rf "$CONDUIT_BUILD_DIR"

# Build and install conduit
mkdir -p "$CONDUIT_BUILD_DIR"
cd "$CONDUIT_BUILD_DIR"
cmake ../src -DCMAKE_INSTALL_PREFIX="$CONDUIT_DIR"
make -j ${MOOSE_JOBS:-4}
make -j ${MOOSE_JOBS:-4} install
