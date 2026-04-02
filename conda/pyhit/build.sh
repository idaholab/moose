#!/bin/bash
set -eu

do_build(){
    rm -rf "${SP_DIR:?}/pyhit/"
    cp -R pyhit "${SP_DIR:?}/"
    cd "${SRC_DIR:?}/src"
    make clean
    make bindings
    install hit.so "${SP_DIR:?}/pyhit/" || return 1
    cat > "${SP_DIR:?}/pyhit-${PKG_VERSION:?}.egg-info" <<FAKE_EGG || exit 1
Metadata-Version: 2.1
Name: pyhit
Version: ${PKG_VERSION}
Summary: MOOSE HIT Parser library
Platform: UNKNOWN
FAKE_EGG
}

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build
