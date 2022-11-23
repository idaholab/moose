#!/bin/bash

set -eu
if [ "$SKIP_DOCS" == "True" ]; then
    export MOOSE_SKIP_DOCS=True
fi
./configure --prefix=${PREFIX}/moose
cd modules/combined

CORES=${MOOSE_JOBS:-2}
make -j $CORES
make install -j $CORES
cd ${PREFIX}/moose/bin
ln -s combined-opt moose-opt

mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export PATH=\${PATH}:${PREFIX}/moose/bin
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
export PATH=\${PATH%":${PREFIX}/moose/bin"}
EOF
