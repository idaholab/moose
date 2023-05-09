#!/bin/bash

set -eu
if [ "$(echo $SKIP_DOCS | tr '[:lower:]' '[:upper:]')" == "TRUE" ]; then
    export MOOSE_SKIP_DOCS=True
fi
./configure --prefix=${PREFIX}/moose
cd modules/combined

CORES=${MOOSE_JOBS:-2}
make -j $CORES
make install -j $CORES
cd ${PREFIX}/moose/bin
ln -s combined-opt moose-opt
ln -s combined-opt moose

# Fix (hack) for moose -> moose symlink collision binary/copy inputs
cd ${PREFIX}/moose/share/moose
for sdir in `ls ../combined`; do
    if [ -d ../combined/$sdir ] && [ ! -d $sdir ] && [ ! -f $sdir ] && [ ! -L $sdir ]; then
        ln -s ../combined/$sdir .
    fi
done

mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export PATH=\${PATH}:${PREFIX}/moose/bin
export MOOSE_BIN=${PREFIX}/moose/bin/moose
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
export PATH=\${PATH%":${PREFIX}/moose/bin"}
unset MOOSE_BIN
EOF
