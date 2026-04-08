#!/bin/bash
set -eux
export PATH=/bin:$PATH
LIBMESH_PREFIX="${PREFIX:?}/moose-libmesh"

function set_libmesh_env(){
    unset LIBMESH_DIR CFLAGS CPPFLAGS CXXFLAGS FFLAGS LIBS \
          LDFLAGS DEBUG_CPPFLAGS DEBUG_CFLAGS DEBUG_CXXFLAGS \
          FORTRANFLAGS DEBUG_FFLAGS DEBUG_FORTRANFLAGS

    if [[ "$(uname)" == Darwin ]]; then
        if [[ $HOST == arm64-apple-darwin* ]]; then
            CTUNING="-march=armv8.3-a -I${PREFIX:?}/include"
            export LIBRARY_PATH="${PREFIX:?}/lib"
        else
            CTUNING="-march=core2 -mtune=haswell"
        fi
    else
        CTUNING="-march=nocona -mtune=haswell"
    fi

    PETSC_DIR="$(pkg-config PETSc --variable=prefix)"
    export PETSC_DIR
    export F90=mpifort
    export F77=mpifort
    export FC=mpifort
    export CC=mpicc
    export CXX=mpicxx
    export CFLAGS="${CTUNING}"
    export CXXFLAGS="${CTUNING}"
    export HYDRA_LAUNCHER=fork
    export INSTALL_BINARY="${SRC_DIR:?}/build-aux/install-sh -C"

    if [[ $HOST == arm64-apple-darwin* ]]; then
        export LDFLAGS="-L${PREFIX:?}/lib -Wl,-rpath,${PREFIX:?}/lib"
    fi
}

function do_build(){
    rm -rf "$LIBMESH_PREFIX"
    mkdir -p "${LIBMESH_PREFIX}/logs" "${SRC_DIR:?}/build"
    cd "${SRC_DIR:?}/build"
    LIBMESH_DIR="$LIBMESH_PREFIX" configure_libmesh \
        --with-vtk-lib="$VTKLIB_DIR" \
        --with-vtk-include="$VTKINCLUDE_DIR"
    cp "${SRC_DIR:?}/build/config.log" "${LIBMESH_PREFIX}/logs/"
    CORES=${MOOSE_JOBS:-6}
    set -o pipefail
    make -j "$CORES" 2>&1 | tee "${LIBMESH_PREFIX}/logs/make.log"
    make install -j "$CORES" 2>&1 | tee "${LIBMESH_PREFIX}/logs/make_install.log"
}

# There are enough "things" we are doing to warrant it's own function
set_libmesh_env

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/configure_libmesh.sh"

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build

# Replace prefix in configs
if [ "$(uname)" = "Darwin" ]; then
    sed -i '' -e "s|${BUILD_PREFIX}|${PREFIX}|g" "$LIBMESH_PREFIX"/bin/libmesh-config
else
    sed -i'' -e "s|${BUILD_PREFIX}|${PREFIX}|g" "$LIBMESH_PREFIX"/bin/libmesh-config

    # Fix hard paths to /usr/bin/ when most operating system want these tools in /bin
    sed -i'' -e "s|/usr/bin/sed|/bin/sed|g" "$LIBMESH_PREFIX"/contrib/bin/libtool
    sed -i'' -e "s|/usr/bin/grep|/bin/grep|g" "$LIBMESH_PREFIX"/contrib/bin/libtool
    sed -i'' -e "s|/usr/bin/dd|/bin/dd|g" "$LIBMESH_PREFIX"/contrib/bin/libtool
fi

# Set LIBMESH_DIR, Eigen3_DIR
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export LIBMESH_DIR=${LIBMESH_PREFIX}
export Eigen3_DIR=${LIBMESH_PREFIX}/include/Eigen
EOF
# Unset previously set variables
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset LIBMESH_DIR
unset Eigen3_DIR
EOF
