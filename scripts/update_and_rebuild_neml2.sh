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
MOOSE_DIR="$( cd "$( dirname "${SCRIPT_DIR}" )" && pwd )"

# Display the help message if requested
if [[ "$#" -eq 1 ]] && [[ "$1" == "--help" ]]; then
  SCRIPT_NAME=$(basename "$0")
  echo "Usage: ${SCRIPT_NAME} --help"
  echo "Usage: ${SCRIPT_NAME} [--skip-submodule-update] [PIP_ARG ...]"
  echo
  echo "Build NEML2 from source and install it (non-editable) into the ACTIVE Python"
  echo "environment -- the same environment that provides PyTorch. A single install"
  echo "provides both the C++ libraries MOOSE links against and the Python package"
  echo "(e.g. neml2-compile, used by the cpp-aoti runtime). The script performs:"
  echo "  1. Update the NEML2 submodule (unless skipped)"
  echo "  2. Check prerequisites (torch, cmake) -- see below"
  echo "  3. Build NEML2 from source and pip-install it into the active environment"
  echo
  echo "Prerequisites in the ACTIVE Python environment. This script CHECKS these but"
  echo "never installs them:"
  echo "  * A neml2-compatible PyTorch (e.g. 'pip install torch'). NEML2 links against"
  echo "    the torch found here; it is deliberately left untouched (your pinned build"
  echo "    is preserved)."
  echo "  * cmake. The minimum version is defined by NEML2 and enforced by its build"
  echo "    backend, so it is not hardcoded here -- only cmake's presence is checked."
  echo "    ninja is recommended but optional."
  echo "  * The scikit-build-core build backend and NEML2's other Python runtime"
  echo "    dependencies. Because the install runs with --no-deps (to protect your"
  echo "    torch), these are not installed for you; if any are missing the script"
  echo "    reports the specific missing module after the build."
  echo
  echo "Influential environment variables:"
  echo "  MOOSE_DIR      The path to the MOOSE directory. Default: the parent of this script."
  echo "  NEML2_SRC_DIR  Path to a custom NEML2 source directory. If set,"
  echo "                 --skip-submodule-update is assumed. Default:"
  echo "                 <MOOSE_DIR>/framework/contrib/neml2."
  echo "  NEML2_JOBS     Number of parallel build jobs. Default: <MOOSE_JOBS>, else 1."
  echo
  echo "Command-line arguments:"
  echo "  --help                   Display this message and exit"
  echo "  --skip-submodule-update  Skip updating the NEML2 submodule (step 1)"
  echo "  PIP_ARG                  Extra argument(s) forwarded to pip verbatim. Both"
  echo "                           '--config-settings=cmake.define.FOO=BAR' and the legacy"
  echo "                           '-DFOO=BAR' forms are accepted (the latter is translated)."
  exit 0
fi

# Handle cliargs
SKIP_SUBMODULE_UPDATE=false
EXTRA_ARGS=()
for ARG in "$@" ; do
  if [[ "${ARG}" == "--skip-submodule-update" ]]; then
    SKIP_SUBMODULE_UPDATE=true
  elif [[ "${ARG}" == -D*=* ]]; then
    # Backward compatibility: translate a raw cmake define into a pip config-setting.
    EXTRA_ARGS+=("--config-settings=cmake.define.${ARG#-D}")
  else
    EXTRA_ARGS+=("$ARG")
  fi
done

# Handle environment variables
if [[ -n "$NEML2_SRC_DIR" ]]; then
  SKIP_SUBMODULE_UPDATE=true
else
  NEML2_SRC_DIR=${MOOSE_DIR}/framework/contrib/neml2
fi

if [[ -z "$NEML2_JOBS" ]]; then
  if [[ -n "$MOOSE_JOBS" ]]; then
    NEML2_JOBS=$MOOSE_JOBS
  else
    # Do not silently saturate the machine: default to a serial build unless the user opts into
    # more parallelism via NEML2_JOBS or MOOSE_JOBS.
    NEML2_JOBS=1
  fi
fi

# Print out the configuration summary
SCRIPT_NAME=$(basename "$0")
echo "****************************************************************************************************"
echo "${SCRIPT_NAME} summary:"
echo "  NEML2_SRC_DIR:             ${NEML2_SRC_DIR}"
echo "  NEML2_JOBS:                ${NEML2_JOBS}"
echo "  SKIP_SUBMODULE_UPDATE:     ${SKIP_SUBMODULE_UPDATE}"
echo "  python3:                   $(command -v python3)"
echo "  Extra pip arguments:       ${EXTRA_ARGS[*]}"
echo "****************************************************************************************************"

# Step 2: Check prerequisites (never installed here). NEML2 v3 links against the torch
# found in the active Python environment, and is built with CMake.
if ! python3 -c 'import torch' >/dev/null 2>&1; then
  echo "Error: 'import torch' failed in the active Python environment."
  echo "NEML2 v3 links against the PyTorch installed in the active environment. Activate an"
  echo "environment that has a compatible torch (e.g. 'pip install torch') and re-run."
  exit 1
fi

# Only cmake's presence is checked -- the minimum version lives in NEML2's pyproject
# (cmake.version) and is enforced by its build backend, so nothing is hardcoded here.
if ! command -v cmake >/dev/null 2>&1; then
  echo "Error: 'cmake' was not found on PATH."
  echo "NEML2 is built with CMake. Install cmake (a recent version -- the required minimum is"
  echo "reported by the NEML2 build itself if the one found is too old) and re-run."
  exit 1
fi

# Step 1: Update the NEML2 submodule
if [[ "$SKIP_SUBMODULE_UPDATE" != true ]]; then
  cd "$MOOSE_DIR" || exit 1
  git submodule update --init --checkout --recursive "${NEML2_SRC_DIR}"
  if [[ $? -ne 0 ]] ; then
    echo "Error: Failed to update the NEML2 submodule with command"
    echo "  git submodule update --init --checkout --recursive ${NEML2_SRC_DIR}"
    exit 1
  fi
fi

# Step 3: Build NEML2 from source and install it into the active environment
# shellcheck disable=SC1091
source "$SCRIPT_DIR/configure_neml2.sh"
echo
echo "****************************************************************************************************"
echo "Building and installing NEML2 into the active Python environment"
echo "****************************************************************************************************"
echo
export CMAKE_BUILD_PARALLEL_LEVEL="${NEML2_JOBS}"
pip_install_neml2 "${NEML2_SRC_DIR}" "${EXTRA_ARGS[@]}"
# shellcheck disable=SC2181
if [[ $? -ne 0 ]] ; then
  echo "Error: Failed to build and install NEML2"
  exit 1
fi

# Resolve the install location (this also verifies NEML2 imports; a failure here
# usually means a Python runtime dependency is missing from the environment).
if ! NEML2_DIR=$(python3 -c 'import neml2, os; print(os.path.dirname(neml2.__file__))' 2>&1); then
  echo
  echo "Error: NEML2 was built and installed, but 'import neml2' failed:"
  echo "${NEML2_DIR}"
  echo
  echo "This usually means a NEML2 Python runtime dependency is missing from the active"
  echo "environment (the traceback above names it). This script installs NEML2 itself but"
  echo "not its dependencies -- so your pinned torch is never disturbed. Install the missing"
  echo "module(s) and re-run."
  exit 1
fi
TORCH_DIR=$(python3 -c 'import torch, os; print(os.path.dirname(torch.__file__))' 2>/dev/null)

echo
echo "****************************************************************************************************"
echo "NEML2 has been successfully installed into:"
echo "  ${NEML2_DIR}"
echo
echo "To configure MOOSE with NEML2, run:"
echo
echo "  cd ${MOOSE_DIR}"
echo "  ./configure --with-neml2=${NEML2_DIR} --with-libtorch=${TORCH_DIR}"
echo
echo "Append other configure options as needed. See ./configure --help for more information."
echo "****************************************************************************************************"
