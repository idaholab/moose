#!/bin/bash
set -eu

# Configure and build options
export INSTALL_PATH=${PREFIX}/seacas
export BUILD=YES
export GNU_PARALLEL=NO
export H5VERSION=V112

# If on macOS, set COMPILER to clang and don't enable X11
if [[ $(uname) == Darwin ]]; then
    export COMPILER=clang
    X11_STR="-DTPL_ENABLE_X11=OFF"
else
    export COMPILER=gcc
    X11_STR="-DTPL_ENABLE_X11=ON"
fi

# Install third party libraries
./install-tpl.sh

# Setup build location and configure there
mkdir build
cd build
../cmake-config $X11_STR

# Make and install
CORES=${MOOSE_JOBS:-2}
make -j $CORES
make install

# Set a SEACAS_DIR environment variable for users who might need it, and add SEACAS bins to the PATH.
# Given SEACAS has its own HDF5, place the bin dir at the end of the PATH, to give priority to
#   moose-mpich HDF5 bins if the user has both packages installed.
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export SEACAS_DIR=${PREFIX}/seacas
export PATH=\${PATH}:${PREFIX}/seacas/bin
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset SEACAS_DIR
export PATH=\${PATH%":${PREFIX}/seacas/bin"}
EOF
