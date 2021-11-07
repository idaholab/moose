#!/bin/bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
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
