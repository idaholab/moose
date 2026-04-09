#!/bin/bash
set -eu

# this function is called from conda/functions/retry_build.sh
function do_build(){
    export INSTALL_PATH="${PREFIX}/moose-seacas"
    rm -rf "${INSTALL_PATH:?}" "${SRC_DIR:?}/build"

    # Configure and build options
    export BUILD=YES
    export GNU_PARALLEL=NO
    export H5VERSION=V112
    export NEEDS_ZLIB=YES
    export JOBS=${MOOSE_JOBS:-2}
    export COMPILER="unused"

    # Install third party libraries
    ./install-tpl.sh

    # Setup build location and configure there.
    mkdir -p "${SRC_DIR:?}/build" || exit 1
    cd "${SRC_DIR:?}/build" || exit 1
    ../cmake-config \
        $CMAKE_ARGS \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_PATH" || return 1
    # Make and install
    cmake --build . --parallel "$JOBS" || return 1
    cmake --install . || return 1
}

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build

# Set a SEACAS_DIR environment variable for users who might need it, and add SEACAS bins to the PATH.
# Given SEACAS has its own HDF5, place the bin dir at the end of the PATH, to give priority to
#   moose-mpich HDF5 bins if the user has both packages installed.
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export SEACAS_DIR=${PREFIX}
export PATH=\${PATH}:${PREFIX}/moose-seacas/bin
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset SEACAS_DIR
export PATH=\${PATH%":${PREFIX}/moose-seacas/bin"}
EOF
