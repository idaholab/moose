#!/bin/bash
set -eux
export PATH=/bin:$PATH
LIBMESH_PREFIX="${PREFIX:?}/moose-libmesh"

function do_build(){
    # Strip flags that cause issues
    STRIP_FLAGS=("-DNDEBUG" "-O2" "-D_FORTIFY_SOURCE=2")
    local _CPPFLAGS="${CPPFLAGS:?} "
    local _CXXFLAGS="${CXXFLAGS:?} "
    local _FFLAGS="${FFLAGS:?} "
    for FLAG in "${STRIP_FLAGS[@]}"; do
        _CPPFLAGS="${_CPPFLAGS/${FLAG} /}"
        _CXXFLAGS="${_CXXFLAGS/${FLAG} /}"
        _FFLAGS="${_FFLAGS/${FLAG} /}"
    done

    rm -rf "$LIBMESH_PREFIX"
    mkdir -p "${SRC_DIR:?}/build"
    cd "${SRC_DIR:?}/build"
    PREFIX="$LIBMESH_PREFIX" LIBMESH_DIR="$LIBMESH_PREFIX" PETSC_DIR="${PETSC_DIR:?}" configure_libmesh \
        --disable-dependency-tracking \
        --with-vtk-lib="${VTKLIB_DIR:?}" \
        --with-vtk-include="${VTKINCLUDE_DIR:?}" \
        CC=mpicc \
        CXX=mpicxx \
        FC=mpifort \
        F77=mpifort \
        F90=mpifort \
        CPPFLAGS="$_CPPFLAGS" \
        CXXFLAGS="$_CXXFLAGS" \
        FFLAGS="$_FFLAGS" || return 1
    make -j "${MOOSE_JOBS:-6}" || return 1
    make install || return 1
}

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/configure_libmesh.sh"

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

# Not sure why, but this resolves issues like "ld: library not found for -lemutls_w"
if [[ $HOST == arm64-apple-darwin* ]]; then
    export LDFLAGS="${LDFLAGS:?} -L${PREFIX:?}/lib -Wl,-rpath,${PREFIX:?}/lib"
fi

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build

# Replace BUILD_PREFIX with installed PREFIX where needed
sedinplace() {
  if [[ $(uname) == Darwin ]]; then
    sed -i "" "$@"
  else
    sed -i"" "$@"
  fi
}
sedinplace s%"${BUILD_PREFIX}"%"${PREFIX}"%g "$LIBMESH_PREFIX"/bin/libmesh-config
sedinplace s%"${BUILD_PREFIX}"%"${PREFIX}"%g "$LIBMESH_PREFIX"/contrib/bin/libtool

# Fix hard paths to /usr/bin/ when most operating system want these tools in /bin
if [ "$(uname)" != "Darwin" ]; then
    for tool in sed grep dd; do
        sed -i"" s%/usr/bin/"${tool}"%/bin/"${tool}"%g "$LIBMESH_PREFIX"/contrib/bin/libtool
    done
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
