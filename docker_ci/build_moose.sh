#!/bin/bash
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script is used in docker_ci/Dockerfile to build
# framework, test, and possibly modules
#
# Used Environment variables:
# METHODS: The methods to build
# MOOSE_JOBS: The number of jobs to run make with
# BUILD_MODULES: If set, build modules
# MOOSE_BUILD_SYSTEM: "make" (default) or "cmake" to build with CMake instead

# Opt-in CMake build (additive; the make path below is unchanged and remains the default).
if [[ "${MOOSE_BUILD_SYSTEM:-make}" == "cmake" ]]; then
  MOOSE_REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
  MOOSE_ENABLE_MODULES_FLAG="-DMOOSE_ENABLE_MODULES=$( [[ -n "$BUILD_MODULES" ]] && echo ON || echo OFF )"
  MOOSE_METHODS="$MOOSE_METHODS" MOOSE_JOBS="$MOOSE_JOBS" \
    "${MOOSE_REPO_DIR}/scripts/build_moose_cmake.sh" ${MOOSE_ENABLE_MODULES_FLAG}
  exit $?
fi

# Runs make in the current directory for all of
# the methods in METHODS, with MOOSE_JOBS jobs
# and any additional arguments
function make_pwd()
{
  for method in $MOOSE_METHODS; do
    METHOD=$method make -j $MOOSE_JOBS $*
  done
}

cd framework
make_pwd

cd ../test
make_pwd

if [[ -n "$BUILD_MODULES" ]]; then
  cd ../modules
  make_pwd all builds
fi
