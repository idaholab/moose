#!/bin/bash
set -eux
export PATH=/bin:$PATH
LIBMESH_PREFIX="${PREFIX:?}/moose-libmesh"

function do_build(){
    rm -rf "$LIBMESH_PREFIX"
    mkdir -p "${SRC_DIR:?}/build"
    cd "${SRC_DIR:?}/build"
    LIBMESH_DIR="$LIBMESH_PREFIX" PETSC_DIR="${PETSC_DIR:?}" configure_libmesh \
        --disable-dependency-tracking \
        --with-vtk-lib="${VTKLIB_DIR:?}" \
        --with-vtk-include="${VTKINCLUDE_DIR:?}"
    set -o pipefail
    make -j "${MOOSE_JOBS:-6}"
    make install
}

# Use MPI wrappers for compilers
export CC=mpicc CXX=mpicxx FC=mpifort F77=mpifort F90=mpifort

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/configure_libmesh.sh"

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

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
