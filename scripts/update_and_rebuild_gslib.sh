#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
set -e
set -u
set -o pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

help=0
skip_sub_update=0
for i in "$@"
do
  shift
  if [[ "$i" == "-h" || "$i" == "--help" ]]; then
    help=1;
  fi

  if [ "$i" == "--skip-submodule-update" ]; then
    skip_sub_update=1
  fi
done

# Display help
if [ $help -eq 1 ]; then
  echo "Usage: $0 [-h | --help | --skip-submodule-update ]"
  echo
  echo "-h | --help              Display this message and list of available libmesh options"
  echo "--skip-submodule-update  Do not update the conduit submodule, use the current version"
  echo
  echo "Influential variables"
  echo "GSLIB_SRC_DIR          Path to gslib; default: ../framework/contrib/gslib from submodule"
  echo "GSLIB_DIR              gslib install prefix; default: ../framework/contrib/gslib/installed"
  exit 0
fi

get_realpath() {
    python3 -c "import os, sys; print(os.path.realpath(sys.argv[1]))" "$1"
}

if [ -v GSLIB_SRC_DIR ]; then
  skip_sub_update=1
else
  GSLIB_SRC_DIR="$(get_realpath "${SCRIPT_DIR}"/../framework/contrib/gslib)"
fi

if [ -v GSLIB_DIR ]; then
  echo "INFO: GSLIB_DIR set - overriding default installed path"
  echo "INFO: No cleaning will be done in specified path"
  GSLIB_DIR="$(get_realpath "${GSLIB_DIR}")"
else
  GSLIB_DIR="${GSLIB_SRC_DIR}/installed"
  # Clean up old builds
  make -C $GSLIB_SRC_DIR clean
  rm -rf "$GSLIB_DIR"
fi

if [ "$skip_sub_update" -eq 1 ]; then
  cd "${SCRIPT_DIR}/.."
  git submodule update --init --checkout framework/contrib/gslib
  cd framework/contrib/gslib
  git submodule update --init
fi

# Default to -O2 and enable PIC
prepend_flags="-O2 -fPIC"
cflags="$prepend_flags ${CFLAGS:-""}"
fflags="$prepend_flags ${FFLAGS:-""}"

# Build and install gslib
cd "$GSLIB_SRC_DIR"
DESTDIR="$GSLIB_DIR" CFLAGS="$cflags" FFLAGS="$fflags" make -j ${MOOSE_JOBS:-4}
