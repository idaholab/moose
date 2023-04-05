#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Configure WASP with the default MOOSE configuration options
#
# Separated so that it can be used across all WASP build scripts:
# - scripts/update_and_rebuild_wasp.sh
# - conda/wasp/build.sh
function configure_wasp()
{
    cmake \
      -D CMAKE_BUILD_TYPE:STRING=RELEASE \
      -D wasp_ENABLE_testframework:BOOL=OFF \
      -D wasp_ENABLE_TESTS:BOOL=OFF \
      -D BUILD_SHARED_LIBS:BOOL=ON \
      -D DISABLE_HIT_TYPE_PROMOTION:BOOL=ON \
      "$@"
}
