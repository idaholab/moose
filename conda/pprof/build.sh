#!/bin/bash

function do_build(){
    export GOPATH="$PREFIX/pprof"
    rm -rf "${GOPATH:?}"
    # Build/Install pprof from google at specified hash
    go install github.com/google/pprof@latest || return 1
    # go creates read-only files. Do this so Civet can properly clean up
    chmod -R 700 "${GOPATH:?}/pkg"
    rm -rf "${GOPATH:?}/pkg"
}

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build

# Set GPERF_DIR path (influential environment variable in MOOSE make files)
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export GPERF_DIR=${PREFIX}
export PPROF_OLDPATH=\${PATH}
export PATH=${PREFIX}/pprof/bin:\${PATH}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset GPERF_DIR
export PATH=\${PPROF_OLDPATH}
EOF
