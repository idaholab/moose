#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
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
  echo "  2. Check prerequisites (torch, cmake), bootstrapping torch if absent -- see below"
  echo "  3. Build NEML2 from source and pip-install it into the active environment"
  echo
  echo "Prerequisites in the ACTIVE Python environment. This script CHECKS these but"
  echo "never installs them, except for the PyTorch bootstrap described below:"
  echo "  * A neml2-compatible PyTorch (e.g. 'pip install torch'). NEML2 links against"
  echo "    the torch found here. An existing torch -- even one that fails to import --"
  echo "    is deliberately left untouched (your pinned build is preserved)."
  echo "    If torch is entirely absent, it is built from the LibTorch build directory"
  echo "    left by update_and_rebuild_libtorch.sh (<LIBTORCH_SRC_DIR>/build) and installed"
  echo "    into the active environment, together with PyTorch's requirements.txt. That"
  echo "    can replace the environment's cmake and ninja with their PyPI builds and"
  echo "    downgrade packages it pins (e.g. setuptools). The existing CMake build is"
  echo "    reconfigured and built incrementally rather than from scratch. Without that"
  echo "    build directory the script stops with an error."
  echo "    Set NEML2_AUTO_INSTALL_TORCH=0 to disable the bootstrap."
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
  echo "  NEML2_AUTO_INSTALL_TORCH"
  echo "                 1 (default) to bootstrap PyTorch when torch is absent; any other"
  echo "                 value (e.g. 0) disables the bootstrap."
  echo "  LIBTORCH_SRC_DIR"
  echo "                 PyTorch source directory whose build/ is reused by the bootstrap."
  echo "                 Default: <MOOSE_DIR>/framework/contrib/pytorch."
  echo "  LIBTORCH_JOBS  Number of parallel jobs for the PyTorch bootstrap. Default: <NEML2_JOBS>."
  echo "  PETSC_DIR      PETSc used by the PyTorch bootstrap for BLAS/LAPACK. Default:"
  echo "                 env:PETSC_DIR, else <MOOSE_DIR>/petsc/arch-moose."
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

# Step 2: Check prerequisites. NEML2 v3 links against the torch found in the active Python
# environment, and is built with CMake.

# Bootstrap Python PyTorch only if the module is completely absent.
# Never overwrite an existing but broken torch installation.
if [[ "${NEML2_AUTO_INSTALL_TORCH:-1}" == "1" ]] &&
   python3 -c 'import importlib.util, sys; sys.exit(0 if importlib.util.find_spec("torch") is None else 1)'; then

  TORCH_SRC_DIR=${LIBTORCH_SRC_DIR:-${MOOSE_DIR}/framework/contrib/pytorch}

  if [[ ! -f "${TORCH_SRC_DIR}/setup.py" ||
        ! -f "${TORCH_SRC_DIR}/build/CMakeCache.txt" ]]; then
    echo "Error: Python torch is missing and no reusable LibTorch build exists."
    echo "Run update_and_rebuild_libtorch.sh first, or install torch into the active environment."
    exit 1
  fi

  # Install with exactly the interpreter the 'import torch' check below uses. Unlike
  # 'command -v python3', sys.executable is absolute, which matters because the install runs
  # from the PyTorch source directory.
  TORCH_PYTHON=$(python3 -c 'import sys; print(sys.executable)') || exit 1

  # install_libtorch_python falls back to the 'pip' on PATH when the interpreter has no pip
  # module (the uv-created apptainer venv, whose pip wrapper targets its own venv). Only accept
  # that fallback when it belongs to the same environment.
  if ! "${TORCH_PYTHON}" -m pip --version &> /dev/null &&
     [[ "$(dirname "$(command -v pip)")" != "$(dirname "${TORCH_PYTHON}")" ]]; then
    echo "Error: ${TORCH_PYTHON} has no pip module and the 'pip' on PATH belongs to another"
    echo "environment; refusing to install PyTorch there."
    exit 1
  fi

  export PETSC_DIR=${PETSC_DIR:-${MOOSE_DIR}/petsc/arch-moose}

  echo
  echo "****************************************************************************************************"
  echo "Python torch is missing; installing PyTorch from ${TORCH_SRC_DIR}/build into ${TORCH_PYTHON}"
  echo "****************************************************************************************************"
  echo

  # Reuse the existing PyTorch installation helper.
  # shellcheck disable=SC1091
  source "${SCRIPT_DIR}/configure_libtorch.sh"

  # PIP_USER=0 stops pip from silently falling back to a user-site install (outside the active
  # environment) when the environment's site-packages is not writable.
  if ! PIP_USER=0 install_libtorch_python \
      "${TORCH_SRC_DIR}" \
      "${TORCH_PYTHON}" \
      "${LIBTORCH_JOBS:-${NEML2_JOBS}}"; then
    echo "Error: Failed to install Python PyTorch."
    exit 1
  fi
fi

if ! python3 -c 'import torch'; then
  echo "Error: 'import torch' failed in the active Python environment."
  echo "NEML2 v3 links against the PyTorch installed in the active environment. Activate an"
  echo "environment that has a compatible torch (e.g. 'pip install torch') and re-run."
  exit 1
fi

# Report the torch NEML2 will build against; cmake_prefix_path is where NEML2's CMake finds Torch.
python3 -c 'import sys, torch, torch.utils
print(f"PyTorch used by NEML2:\n  python3:                   {sys.executable}")
print(f"  torch.__file__:            {torch.__file__}")
print(f"  torch.__version__:         {torch.__version__}")
print(f"  torch cmake_prefix_path:   {torch.utils.cmake_prefix_path}")'

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
