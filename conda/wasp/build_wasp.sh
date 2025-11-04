#!/bin/bash
set -eu
export PATH=/bin:$PATH

function do_build(){
    rm -rf "${SRC_DIR:?}/build" "${PREFIX:?}/wasp"
    mkdir "${SRC_DIR:?}/build" || exit 1
    cd "${SRC_DIR:?}/build" || exit 1

    local WASP_OPTIONS="-D CMAKE_INSTALL_PREFIX:STRING=${PREFIX}/wasp"
    # shellcheck disable=SC1091  # made available through meta.yaml src path
    source "${SRC_DIR}/configure_wasp.sh"
    configure_wasp "$WASP_OPTIONS" ../ || return 1
    make -j "${MOOSE_JOBS:-2}" install || return 1
}

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build

# Set WASP_DIR environment variable(s)
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export WASP_DIR=${PREFIX}/wasp
export PATH=\${PATH}:${PREFIX}/wasp/bin
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset WASP_DIR
export PATH=\${PATH%":${PREFIX}/wasp/bin"}
EOF
