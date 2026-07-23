#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Build NEML2 from source and install it (non-editable) into the ACTIVE Python
# environment's site-packages -- the same environment that provides PyTorch. A
# single pip install yields BOTH the C++ artifacts MOOSE links against
# (libneml2*.so, neml2.pc, public headers) AND the Python package (e.g.
# neml2-compile, used by the cpp-aoti runtime).
#
# Separated so that it can be reused across NEML2 build scripts:
# - scripts/update_and_rebuild_neml2.sh
#
# Arguments:
#   1. Path to the NEML2 source directory
#   Remaining arguments are passed to pip verbatim (e.g.
#   --config-settings=cmake.define.FOO=BAR).
function pip_install_neml2()
{
  local src_dir="$1"
  shift
  # --no-build-isolation : the build must see the active env's torch (torch is a
  #                        runtime dep of NEML2, not a build requirement, so an
  #                        isolated build env would not have it).
  # --no-deps            : install NEML2 itself only. This leaves the environment's
  #                        dependencies -- most importantly the user's pinned torch
  #                        build -- untouched (a plain --force-reinstall would
  #                        otherwise re-resolve and clobber torch).
  # --force-reinstall
  # --no-cache-dir       : guarantee a genuine rebuild+reinstall even when the
  #                        source changed but the version string did not (pip's
  #                        wheel cache would otherwise serve a stale build).
  # NEML2_MPI=ON is the one build option MOOSE requires; any additional cmake
  # options are forwarded by the caller as further --config-settings arguments.
  python3 -m pip install "$src_dir" \
    --no-build-isolation \
    --no-deps \
    --force-reinstall \
    --no-cache-dir \
    --config-settings=cmake.define.NEML2_MPI=ON \
    "$@"
}
