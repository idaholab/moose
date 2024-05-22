#!/bin/bash
set -eu
export PATH=/bin:$PATH

rm -rf build; mkdir build; cd build
WASP_OPTIONS="-D CMAKE_INSTALL_PREFIX:STRING=${PREFIX}/wasp"
source $SRC_DIR/configure_wasp.sh
configure_wasp "$WASP_OPTIONS" ../

CORES=${MOOSE_JOBS:-2}
make -j $CORES install

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
