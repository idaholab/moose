#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

function get_variable()
{
  if [ -z "${!1}" ]; then
    >&2 echo "ERROR: Missing required variable $1"
  fi
  echo "${!1}"
}

# Configure NEML2 with the default MOOSE configuration options
#
# Separated so that it can be used across all NEML2 build scripts:
# - scripts/update_and_rebuild_wasp.sh
#
# Arguments:
#   1. Path to the NEML2 source directory
#   2. Path to the NEML2 build directory
#
# Remaining arguments will be appended to the cmake command verbatim
function configure_neml2()
{
  ARGS=()
  if [ -n "$HIT_SRC_DIR" ]; then
    ARGS+=("-Dhit_SOURCE_DIR=$HIT_SRC_DIR")
  fi
  if which ninja &> /dev/null; then
    ARGS+=("-GNinja")
  fi
  ARGS+=("${@:3}")
  cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DNEML2_TESTS=OFF \
    -DNEML2_RUNNER=OFF \
    -DNEML2_PYBIND=OFF \
    -DNEML2_DOC=OFF \
    -DNEML2_WORK_DISPATCHER=ON \
    -DNEML2_JSON=OFF \
    -DNEML2_CONTRIB_PREFIX="$2/contrib" \
    -Dtorch_ROOT="$(get_variable LIBTORCH_DIR)" \
    -Dwasp_ROOT="$(get_variable WASP_DIR)" \
    -Dtimpi_ROOT="$(get_variable TIMPI_DIR)" \
    -G "Unix Makefiles" \
    -B "$2" \
    -S "$1" \
    "${ARGS[@]}"
}

# Build NEML2 assuming that the project has already been configured
#
# Arguments:
#   1. Path to the NEML2 build directory
#   2. Number of jobs to use when building NEML2
function build_neml2()
{
  cmake --build "$1" --parallel "$2" --target all
}

# Install NEML2
#
# Arguments:
#   1. Path to the NEML2 build directory
#   2. Install prefix
function install_neml2()
{
  cmake --install "$1" --prefix "$2"
}
