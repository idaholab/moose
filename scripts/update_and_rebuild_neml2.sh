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
  echo "This script makes NEML2 available to the MOOSE framework. The script "
  echo "performs the following steps:"
  echo "  1. Update the NEML2 submodule"
  echo "  2. Clean and recreate the build directory"
  echo "  3. Configure NEML2"
  echo "  4. Build NEML2"
  echo "  5. Install NEML2"
  echo
  echo "NEML2 requires TIMPI, WASP, and libtorch as dependencies. They can be "
  echo "obtained using the following scripts:"
  echo "  - TIMPI:    scripts/update_and_rebuild_libmesh.sh"
  echo "  - WASP:     scripts/update_and_rebuild_wasp.sh"
  echo "  - libtorch: scripts/setup_libtorch.sh"
  echo
  echo "Influential environment variables:"
  echo "  MOOSE_DIR       The path to the MOOSE directory. Default to the parent directory of this script."
  echo "  LIBMESH_SRC_DIR The path to the libMesh source directory. Default to <MOOSE_DIR>/libmesh."
  echo "  LIBMESH_DIR     The path to the libMesh directory. Default to <LIBMESH_SRC_DIR>/installed."
  echo "  TIMPI_DIR       The path to the TIMPI directory. Default to <LIBMESH_DIR>."
  echo "  WASP_SRC_DIR    The path to the WASP source directory. Default to <MOOSE_DIR>/framework/contrib/wasp."
  echo "  WASP_DIR        The path to the WASP directory. Default to <WASP_SRC_DIR>/install."
  echo "  LIBTORCH_DIR    The path to the libtorch directory. Default to <MOOSE_DIR>/framework/contrib/libtorch."
  echo "  NEML2_DIR       The path where to install NEML2. Default to <NEML2_SRC_DIR>/installed."
  echo "  NEML2_SRC_DIR   The path to the NEML2 source directory if a custom NEML2 should be used. If set, "
  echo "                  --skip-submodule-update will be assumed."
  echo "  NEML2_JOBS      The number of jobs to use when building NEML2. Default to <MOOSE_JOBS>. "
  echo "                  If unset, default to 1."
  echo "  METHODS         A list of (comma-separated) methods to use for configuring NEML2. Default to opt,dbg."
  echo "  METHOD          The method to use for configuring NEML2. Setting this variable overrides METHODS."
  echo
  echo "General environment variables supported by CMake are also respected."
  echo
  echo "Command-line arguments:"
  echo "  --help                   Display this message and exit"
  echo "  --fast                   Skip the update, clean, and configure steps (steps 1-3)"
  echo "  --skip-submodule-update  Skip the update step (step 1)"
  echo "  ADDITIONAL_CONFIGURE_ARG Additional argument(s) to pass to the NEML2 cmake configure command"
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

# Dependency: libtorch
export LIBTORCH_DIR=${LIBTORCH_DIR:-${MOOSE_DIR}/framework/contrib/libtorch}

# Dependency: timpi
LIBMESH_SRC_DIR=${LIBMESH_SRC_DIR:-${MOOSE_DIR}/libmesh}
LIBMESH_DIR=${LIBMESH_DIR:-${LIBMESH_SRC_DIR}/installed}
export TIMPI_DIR=${TIMPI_DIR:-${LIBMESH_DIR}}

# Dependency: wasp
WASP_SRC_DIR=${WASP_SRC_DIR:-${MOOSE_DIR}/framework/contrib/wasp}
export WASP_DIR=${WASP_DIR:-${WASP_SRC_DIR}/install}

# Dependency: hit in place if it exists
if [ -z "$HIT_SRC_DIR" ] && [ -d "${MOOSE_DIR}/framework/contrib/hit" ]; then
  HIT_SRC_DIR=${MOOSE_DIR}/framework/contrib/hit
fi
export HIT_SRC_DIR

# Handle environment variables
if [[ -n "$NEML2_SRC_DIR" ]]; then
  SKIP_SUBMODULE_UPDATE=true
else
  NEML2_SRC_DIR=${MOOSE_DIR}/framework/contrib/neml2
fi
NEML2_DIR=${NEML2_DIR:-${NEML2_SRC_DIR}/installed}

if [[ -z "$NEML2_JOBS" ]]; then
  if [[ -n "$MOOSE_JOBS" ]]; then
    NEML2_JOBS=$MOOSE_JOBS
  else
    NEML2_JOBS=1
  fi
fi

# Dynamic library suffix
if [[ $(uname) == "Darwin" ]]; then
  DYLIB_SUFFIX=dylib
else
  DYLIB_SUFFIX=so
fi

# Build methods
#
# Note that the build directory is created under the NEML2 source directory,
# and several sub-directories are created each corresponding to a different build
# METHOD. See below for the mapping between METHODS and cmake build types.
#
# opt   <--> Release
# devel <--> RelWithDebInfo
# dbg   <--> Debug
# oprof <--> Profiling
METHODS=${METHODS:-opt,dbg}
if [[ -n "$METHOD" ]]; then
  METHODS=$METHOD
fi

# Print out the configuration summary if requested
SCRIPT_NAME=$(basename "$0")
echo "****************************************************************************************************"
echo "${SCRIPT_NAME} summary:"
echo "  HIT_SRC_DIR:               ${HIT_SRC_DIR}"
echo "  TIMPI_DIR:                 ${TIMPI_DIR}"
echo "  WASP_DIR:                  ${WASP_DIR}"
echo "  LIBTORCH_DIR:              ${LIBTORCH_DIR}"
echo "  NEML2_DIR:                 ${NEML2_DIR}"
echo "  NEML2_SRC_DIR:             ${NEML2_SRC_DIR}"
echo "  NEML2_JOBS:                ${NEML2_JOBS}"
echo "  METHODS:                   ${METHODS}"
echo "  FAST:                      ${FAST}"
echo "  SKIP_SUBMODULE_UPDATE:     ${SKIP_SUBMODULE_UPDATE}"
echo "  ADDITIONAL_CONFIGURE_ARGS: ${EXTRA_ARGS[*]}"
echo "****************************************************************************************************"

# Check that dependencies are available
if [[ ! -d "${LIBTORCH_DIR}" ]]; then
  echo "Error: The libtorch directory (${LIBTORCH_DIR}) does not exist. Please see --help for more information."
  exit 1
fi
if [[ ! -d "${TIMPI_DIR}" ]]; then
  echo "Error: The TIMPI directory (${TIMPI_DIR}) does not exist. Please see --help for more information."
  exit 1
fi
if [[ ! -d "${WASP_DIR}" ]]; then
  echo "Error: The WASP directory (${WASP_DIR}) does not exist. Please see --help for more information."
  exit 1
fi
if [[ ! -f "${WASP_DIR}"/lib/libwaspcore.${DYLIB_SUFFIX} ]] || [[ ! -f "${WASP_DIR}"/lib/libwasphit.${DYLIB_SUFFIX} ]]; then
  echo "Error: The WASP directory (${WASP_DIR}) does not contain required libraries (core and hit). Please build WASP first."
  exit 1
fi

# Step 1: Update the NEML2 submodule
if [[ "$SKIP_SUBMODULE_UPDATE" != true ]] && [[ "$FAST" != true ]]; then
  cd "$MOOSE_DIR" || exit
  git submodule update --init --checkout "${NEML2_SRC_DIR}"
  if [[ $? -ne 0 ]] ; then
    echo "Error: Failed to update the NEML2 submodule with command"
    echo "  git submodule update --init --checkout ${NEML2_SRC_DIR}"
    exit 1
  fi
fi

# Loop over the methods to configure, build, and install NEML2
for METHOD in $(echo "$METHODS" | tr ',' ' '); do
  # Check that timpi is available
  if [[ ! -f "${TIMPI_DIR}"/lib/libtimpi_${METHOD}.${DYLIB_SUFFIX} ]]; then
    echo "Error: The TIMPI library (${TIMPI_DIR}/lib/libtimpi_${METHOD}.${DYLIB_SUFFIX}) does not exist. Please build libMesh first."
    exit 1
  fi

  # Build and install directories
  NEML2_BUILD_DIR=${NEML2_SRC_DIR}/build/${METHOD}

  # If we are going fast, the build directory must already exist
  if [[ "${FAST}" == true ]] && [[ ! -d "${NEML2_BUILD_DIR}" ]] ; then
    echo "Error: A build directory (${NEML2_BUILD_DIR}) must exist to use the --fast option"
    exit 1
  fi

  # CMake build type
  if [[ ${METHOD} == "opt" ]]; then
    CMAKE_BUILD_TYPE="Release"
  elif [[ ${METHOD} == "devel" ]]; then
    CMAKE_BUILD_TYPE="RelWithDebInfo"
  elif [[ ${METHOD} == "dbg" ]]; then
    CMAKE_BUILD_TYPE="Debug"
  elif [[ ${METHOD} == "oprof" ]]; then
    CMAKE_BUILD_TYPE="Profiling"
  else
    echo "Error: Unknown build method ${METHOD}"
    exit 1
  fi

  # Step 2: Clean and recreate the build directory
  if [[ "${FAST}" != true  ]] ; then
    rm -rf "${NEML2_BUILD_DIR}"
    mkdir -p "${NEML2_BUILD_DIR}"
  fi

  # Step 3: Configure NEML2
  source $SCRIPT_DIR/configure_neml2.sh
  if [[ "${FAST}" != true  ]] ; then
    echo
    echo "****************************************************************************************************"
    echo "Configuring NEML2 for METHOD=${METHOD}"
    echo "****************************************************************************************************"
    echo
    configure_neml2 "${NEML2_SRC_DIR}" \
                    "${NEML2_BUILD_DIR}" \
                    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
                    "${EXTRA_ARGS[@]}"
    if [[ $? -ne 0 ]] ; then
      echo "Error: Failed to configure NEML2"
      exit 1
    fi
  fi

  # Step 4: Build NEML2
  echo
  echo "****************************************************************************************************"
  echo "Compiling NEML2 for METHOD=${METHOD}"
  echo "****************************************************************************************************"
  echo
  build_neml2 "${NEML2_BUILD_DIR}" "${NEML2_JOBS}"
  if [[ $? -ne 0 ]] ; then
    echo "Error: Failed to build NEML2"
    exit 1
  fi

  # Step 5: Install NEML2
  echo
  echo "****************************************************************************************************"
  echo "Installing NEML2 for METHOD=${METHOD}"
  echo "****************************************************************************************************"
  echo
  install_neml2 "${NEML2_BUILD_DIR}" "${NEML2_DIR}"
  if [[ $? -ne 0 ]] ; then
    echo "Error: Failed to install NEML2"
    exit 1
  fi
done

if [[ $? -eq 0 ]]; then
  echo
  echo "****************************************************************************************************"
  echo "NEML2 has been successfully installed. "
  echo
  echo "To configure MOOSE with NEML2, run the following commands:"
  echo "  cd ${MOOSE_DIR}"
  echo "  ./configure --with-neml2=${NEML2_DIR} --with-libtorch=${LIBTORCH_DIR}"
  echo "****************************************************************************************************"
fi
