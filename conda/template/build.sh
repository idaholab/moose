#!/bin/bash
set -eux
export APPLICATION=<APPLICATION>
export MOOSE_JOBS=<MOOSE_JOBS>
export SKIP_DOCS=<SKIP_DOCS>
export O_WORKDIR=`pwd`
export INSTALL_DIR=${PREFIX}/${APPLICATION}

#### APPLICATION
export EXTERNAL_FLAGS="-Wl,-headerpad_max_install_names"
export PYTHONPATH=${O_WORKDIR}<MOOSE>/python
if [ "$SKIP_DOCS" == "True" ]; then
    export MOOSE_SKIP_DOCS="True"
fi
cd ${O_WORKDIR}<MOOSE>
./configure --prefix=${INSTALL_DIR}
cd ${O_WORKDIR}<IS_MOOSE>
make -j <MOOSE_JOBS>
make install -j <MOOSE_JOBS>
cd ${PREFIX}/bin
ln -s ${INSTALL_DIR}/bin/<EXECUTABLE>-opt .
ln -s ${INSTALL_DIR}/bin/moose_test_runner .
cd ${PREFIX}/share
ln -s ${INSTALL_DIR}/share/<APPLICATION> .
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export MOOSE_ADFPARSER_JIT_INCLUDE=${INSTALL_DIR}/include/moose/ADRealMonolithic.h
export <FORMATTED_APPLICATION>_DOCS=${INSTALL_DIR}/share/<APPLICATION>/doc/index.html
export NCRC_APP=<FORMATTED_APPLICATION>
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset MOOSE_ADFPARSER_JIT_INCLUDE
unset <FORMATTED_APPLICATION>_DOCS
unset NCRC_APP
EOF
if [ ${APPLICATION} == "moose" ]; then
    cat <<EOF >> "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
alias moose-opt="combined-opt"
EOF
    cat <<EOF >> "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unalias moose-opt
EOF
fi
