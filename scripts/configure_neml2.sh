#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Configure NEML2 with the default MOOSE configuration options
#
# Separated so that it can be used across all NEML2 build scripts:
# - scripts/update_and_rebuild_neml2.sh
#
# Arguments:
#   1. Path to the NEML2 source directory
#   2. Path to the NEML2 build directory
#
# Remaining arguments will be appended to the cmake command verbatim
function configure_neml2()
{
  ARGS=()
  if which ninja &> /dev/null; then
    ARGS+=("-GNinja")
  fi
  ARGS+=("${@:3}")
  # NEML2 v3 discovers torch (and nmhit) from the active Python's site-packages.
  # We pin Python3_EXECUTABLE so Findtorch.cmake / Findnmhit.cmake resolve the
  # torch and nmhit installed in the current (e.g. conda) environment.
  cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DNEML2_CONTRIB_PREFIX="$2/contrib" \
    -DNEML2_MPI=ON \
    -DPython3_EXECUTABLE="$(command -v python3)" \
    -Dtorch_SEARCH_SITE_PACKAGES=ON \
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
