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
  echo "Usage: ${SCRIPT_NAME} [--fast|--skip-submodule-update] [ADDITIONAL_CONFIGURE_ARG ...]"
  echo
  echo "This script makes libtorch available to the MOOSE framework. The script "
  echo "performs the following steps:"
  echo "  1. Update the libtorch submodule"
  echo "  2. Clean and recreate the build directory"
  echo "  3. Configure libtorch"
  echo "  4. Build libtorch"
  echo "  5. Install libtorch"
  echo
  echo
  echo "Influential environment variables:"
  echo "  MOOSE_DIR         The path to the MOOSE directory. Default to the parent directory of this script."
  echo "  PETSC_DIR         The path to the PETSc installation directory. Default to env:PETSC_DIR or "
  echo "                    <MOOSE_DIR>/petsc/arch-moose."
  echo "  LIBTORCH_SRC_DIR  The path to the libtorch source directory if a custom libtorch should be used. If set, "
  echo "                    --skip-submodule-update will be assumed."
  echo "  LIBTORCH_DIR      The path to the libtorch installation directory. Default to "
  echo "                    <MOOSE_DIR>/framework/contrib/libtorch/installed."
  echo "  LIBTORCH_JOBS     The number of jobs to use when building libtorch. Default to <MOOSE_JOBS>. "
  echo "                    If unset, default to 1."
  echo
  echo "For Intel GPU support, please explicitly `export USE_XPU=1` before running the script."
  echo
  echo "General environment variables supported by CMake are also respected."
  echo
  echo "Command-line arguments:"
  echo "  --help                   Display this message and exit"
  echo "  --fast                   Skip the update, clean, and configure steps (steps 1-3)"
  echo "  --skip-submodule-update  Skip the update step (step 1)"
  echo "  ADDITIONAL_CONFIGURE_ARG Additional argument(s) to pass to the libtorch cmake configure command"
  exit 0
fi

# Handle cliargs
FAST=false
SKIP_SUBMODULE_UPDATE=false
for ARG in "$@" ; do
  if [[ "${ARG}" == "--fast" ]]; then
    FAST=true
  elif [[ "${ARG}" == "--skip-submodule-update" ]]; then
    SKIP_SUBMODULE_UPDATE=true
  else
    EXTRA_ARGS+=("$ARG")
  fi
done

# Handle environment variables
if [[ -n "$LIBTORCH_SRC_DIR" ]]; then
  SKIP_SUBMODULE_UPDATE=true
else
  LIBTORCH_SRC_DIR=${MOOSE_DIR}/framework/contrib/pytorch
fi
LIBTORCH_DIR=${LIBTORCH_DIR:-${LIBTORCH_SRC_DIR}/installed}

if [[ -z "$LIBTORCH_JOBS" ]]; then
  if [[ -n "$MOOSE_JOBS" ]]; then
    LIBTORCH_JOBS=$MOOSE_JOBS
  else
    LIBTORCH_JOBS=1
  fi
fi

# Dependency: petsc
export PETSC_DIR=${PETSC_DIR:-${MOOSE_DIR}/petsc/arch-moose}

# Print out the configuration summary if requested
SCRIPT_NAME=$(basename "$0")
echo "****************************************************************************************************"
echo "${SCRIPT_NAME} summary:"
echo "  MOOSE_DIR:                 ${MOOSE_DIR}"
echo "  PETSC_DIR:                 ${PETSC_DIR}"
echo "  LIBTORCH_DIR:              ${LIBTORCH_DIR}"
echo "  LIBTORCH_SRC_DIR:          ${LIBTORCH_SRC_DIR}"
echo "  LIBTORCH_JOBS:             ${LIBTORCH_JOBS}"
echo "  FAST:                      ${FAST}"
echo "  SKIP_SUBMODULE_UPDATE:     ${SKIP_SUBMODULE_UPDATE}"
echo "  ADDITIONAL_CONFIGURE_ARGS: ${EXTRA_ARGS[*]}"
echo "****************************************************************************************************"


# Check that dependencies are available
if [[ ! -d "${PETSC_DIR}" ]]; then
  echo "Error: The PETSc directory (${PETSC_DIR}) does not exist. Please see --help for more information."
  exit 1
fi

# Step 1: Update the LIBTORCH submodule
if [[ "$SKIP_SUBMODULE_UPDATE" != true ]] && [[ "$FAST" != true ]]; then
  cd "$MOOSE_DIR" || exit
  git submodule update --init --checkout --recursive "${LIBTORCH_SRC_DIR}"
  if [[ $? -ne 0 ]] ; then
    echo "Error: Failed to update the libtorch submodule with command"
    echo "  git submodule update --init --checkout --recursive ${LIBTORCH_SRC_DIR}"
    exit 1
  fi
fi

# Build and install directories
LIBTORCH_BUILD_DIR=${LIBTORCH_SRC_DIR}/build

# If we are going fast, the build directory must already exist
if [[ "${FAST}" == true ]] && [[ ! -d "${LIBTORCH_BUILD_DIR}" ]] ; then
  echo "Error: A build directory (${LIBTORCH_BUILD_DIR}) must exist to use the --fast option"
  exit 1
fi

# Step 2: Clean and recreate the build directory
if [[ "${FAST}" != true  ]] ; then
  rm -rf "${LIBTORCH_BUILD_DIR}"
  mkdir -p "${LIBTORCH_BUILD_DIR}"
fi

# Step 3: Configure
# shellcheck disable=SC1091
source "$SCRIPT_DIR/configure_libtorch.sh"
if [[ "${FAST}" != true  ]] ; then
  echo
  echo "****************************************************************************************************"
  echo "Configuring libtorch"
  echo "****************************************************************************************************"
  echo
  configure_libtorch "${LIBTORCH_SRC_DIR}" \
                     "${LIBTORCH_BUILD_DIR}" \
                     "${LIBTORCH_DIR}" \
                     "${EXTRA_ARGS[@]}"
  # shellcheck disable=SC2181
  if [[ $? -ne 0 ]] ; then
    echo "Error: Failed to configure libtorch"
    exit 1
  fi
fi

# Step 4: Build
echo
echo "****************************************************************************************************"
echo "Compiling libtorch"
echo "****************************************************************************************************"
echo
build_libtorch "${LIBTORCH_BUILD_DIR}" "${LIBTORCH_JOBS}"
# shellcheck disable=SC2181
if [[ $? -ne 0 ]] ; then
  echo "Error: Failed to build libtorch"
  exit 1
fi

# Step 5: Install
echo
echo "****************************************************************************************************"
echo "Installing libtorch"
echo "****************************************************************************************************"
echo
install_libtorch "${LIBTORCH_BUILD_DIR}" "${LIBTORCH_DIR}"
# shellcheck disable=SC2181
if [[ $? -ne 0 ]] ; then
  echo "Error: Failed to install libtorch"
  exit 1
fi

# shellcheck disable=SC2181
if [[ $? -eq 0 ]]; then
  echo
  echo "****************************************************************************************************"
  echo "libtorch has been successfully installed. "
  echo
  echo "To configure MOOSE with libtorch, run the following commands:"
  echo
  echo "  cd ${MOOSE_DIR}"
  echo "  ./configure --with-libtorch=${LIBTORCH_DIR}"
  echo
  echo "Append other configure options as needed. See configure --help for more information."
  echo "****************************************************************************************************"
fi
