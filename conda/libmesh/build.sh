#!/bin/bash
set -eu
export PATH=/bin:$PATH
export PKG_CONFIG_PATH=${BUILD_PREFIX:?}/lib/pkgconfig:${PKG_CONFIG_PATH}

function set_libmesh_env(){
    unset LIBMESH_DIR CFLAGS CPPFLAGS CXXFLAGS FFLAGS LIBS \
          LDFLAGS DEBUG_CPPFLAGS DEBUG_CFLAGS DEBUG_CXXFLAGS \
          FORTRANFLAGS DEBUG_FFLAGS DEBUG_FORTRANFLAGS

    if [[ "$(uname)" == Darwin ]]; then
        if [[ $HOST == arm64-apple-darwin20.0.0 ]]; then
            CTUNING="-march=armv8.3-a -I${PREFIX:?}/include"
            export LIBRARY_PATH="${PREFIX:?}/lib"
        else
            CTUNING="-march=core2 -mtune=haswell"
        fi
    else
        CTUNING="-march=nocona -mtune=haswell"
    fi

    PETSC_DIR="$(pkg-config PETSc --variable=prefix)"
    LIBMESH_DIR="${PREFIX:?}/libmesh"
    export PETSC_DIR
    export LIBMESH_DIR
    export F90=mpifort
    export F77=mpifort
    export FC=mpifort
    export CC=mpicc
    export CXX=mpicxx
    export CFLAGS="${CTUNING}"
    export CXXFLAGS="${CTUNING}"
    export HYDRA_LAUNCHER=fork
    export INSTALL_BINARY="${SRC_DIR:?}/build-aux/install-sh -C"

    if [[ $HOST == arm64-apple-darwin20.0.0 ]]; then
        LDFLAGS="-L${PREFIX:?}/lib -Wl,-S,-rpath,${PREFIX:?}/lib"
    else
        export LDFLAGS="-Wl,-S"
    fi
}

function do_build(){
    rm -rf "${LIBMESH_DIR:?}"
    mkdir -p "${SRC_DIR:?}/build"; cd "${SRC_DIR:?}/build"
    configure_libmesh --with-vtk-lib="${BUILD_PREFIX}"/libmesh-vtk/lib \
                      --with-vtk-include="${BUILD_PREFIX}"/libmesh-vtk/include/vtk-"${VTK_VERSION}"
    CORES=${MOOSE_JOBS:-6}
    make -j "$CORES"
    make install -j "$CORES"
}

function sed_replace(){
    if [ "$(uname)" = "Darwin" ]; then
        sed -i '' -e "s|${BUILD_PREFIX}|${PREFIX}|g" "$PREFIX"/libmesh/bin/libmesh-config
    else
        sed -i'' -e "s|${BUILD_PREFIX}|${PREFIX}|g" "$PREFIX"/libmesh/bin/libmesh-config

        # Fix hard paths to /usr/bin/ when most operating system want these tools in /bin
        sed -i'' -e "s|/usr/bin/sed|/bin/sed|g" "$PREFIX"/libmesh/contrib/bin/libtool
        sed -i'' -e "s|/usr/bin/grep|/bin/grep|g" "$PREFIX"/libmesh/contrib/bin/libtool
        sed -i'' -e "s|/usr/bin/dd|/bin/dd|g" "$PREFIX"/libmesh/contrib/bin/libtool
    fi
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

sed_replace

# Set LIBMESH_DIR, Eigen3_DIR
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export LIBMESH_DIR=${PREFIX}/libmesh
export Eigen3_DIR=${PREFIX}/libmesh/include/Eigen
EOF
# Unset previously set variables
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset LIBMESH_DIR
unset Eigen3_DIR
EOF
