#!/bin/bash
set -eu
export PATH=/bin:$PATH

export CC=$(basename "$CC")
export CXX=$(basename "$CXX")
export FC=$(basename "$FC")
unset CPPFLAGS CFLAGS CXXFLAGS FFLAGS FCFLAGS F90 F77
if [[ $(uname) == Darwin ]]; then
    SHARED=clang
    TUNING="-march=core2 -mtune=haswell"
else
    SHARED=gcc
    TUNING="-march=nocona -mtune=haswell"
fi

./configure --prefix=$PREFIX \
            --enable-shared \
            --enable-sharedlibs=$SHARED \
            --enable-fast=O2 \
            --enable-debuginfo \
            --enable-two-level-namespace \
            CC=$CC CXX=$CXX FC=$FC F77=$FC F90='' \
            CFLAGS="${TUNING}" CXXFLAGS="${TUNING}" FFLAGS="${TUNING}" LDFLAGS="${LDFLAGS:-}" \
            FCFLAGS="${TUNING}" F90FLAGS='' F77FLAGS=''

make -j"${CPU_COUNT:-1}"
make install

# Set PETSC_DIR environment variable for those that need it
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export MPIHOME=${PREFIX}
export MOOSE_NO_CODESIGN=true
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset MPIHOME
unset MOOSE_NO_CODESIGN CCACHE_SLOPPINESS CC CXX FC F90 F77
EOF
