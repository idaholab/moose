#!/bin/bash
set -eux

# shellcheck disable=SC1073
# shellcheck disable=SC1009
# shellcheck disable=SC1072  # these values are templated
export APPLICATION=<APPLICATION>
export MOOSE_JOBS=<MOOSE_JOBS>
export SKIP_DOCS=<SKIP_DOCS>
export MOOSE_OPTIONS=<MOOSE_OPTIONS>
export INSTALL_DIR="${PREFIX:?}/${APPLICATION:?}"

function do_build(){
    rm -rf "${INSTALL_DIR:?}"

    export EXTERNAL_FLAGS="-Wl,-headerpad_max_install_names"
    export PYTHONPATH=${SRC_DIR:?}<MOOSE>/python
    if [[ "${SKIP_DOCS}" == "True" ]]; then
        export MOOSE_SKIP_DOCS="True"
    fi
    cd "${SRC_DIR:?}"
    git clean -xfd; git submodule foreach --recursive git clean -xfd
    cd "${SRC_DIR:?}<MOOSE>"
    ./configure --prefix=${INSTALL_DIR:?} ${MOOSE_OPTIONS}
    cd "${SRC_DIR:?}<IS_MOOSE>"
    make -j <MOOSE_JOBS> || return 1
    make install -j <MOOSE_JOBS> || return 1
}

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build

cd "${PREFIX:?}/bin"
ln -s ${INSTALL_DIR:?}/bin/<EXECUTABLE>-opt .
ln -s ${INSTALL_DIR:?}/bin/moose_test_runner .
cd "${PREFIX:?}/share"
if ! [[ -d "<APPLICATION>" ]]; then
    ln -s "${INSTALL_DIR:?}/share/<APPLICATION>" .
fi
mkdir -p "${PREFIX:?}/etc/conda/activate.d" "${PREFIX:?}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX:?}/etc/conda/activate.d/activate_${PKG_NAME:?}.sh"
export MOOSE_ADFPARSER_JIT_INCLUDE=${INSTALL_DIR}/include/moose/ADRealMonolithic.h
export <FORMATTED_APPLICATION>_DOCS=${INSTALL_DIR}/share/<APPLICATION>/doc/index.html
export NCRC_APP=<FORMATTED_APPLICATION>
EOF
cat <<EOF > "${PREFIX:?}/etc/conda/deactivate.d/deactivate_${PKG_NAME:?}.sh"
unset MOOSE_ADFPARSER_JIT_INCLUDE
unset <FORMATTED_APPLICATION>_DOCS
unset NCRC_APP
EOF
if [[ "${APPLICATION:?}" == "moose" ]]; then
    cat <<EOF >> "${PREFIX:?}/etc/conda/activate.d/activate_${PKG_NAME:?}.sh"
alias moose-opt="combined-opt"
EOF
    cat <<EOF >> "${PREFIX:?}/etc/conda/deactivate.d/deactivate_${PKG_NAME:?}.sh"
unalias moose-opt
EOF
fi
