#!/bin/bash
set -eu

function do_build(){
    rm -rf "${PREFIX:?}/moose"
    if [[ "$(echo "${SKIP_DOCS}" | tr '[:lower:]' '[:upper:]')" == "TRUE" ]]; then
        export MOOSE_SKIP_DOCS=True
    fi
    # shellcheck disable=SC2086  # we want word spliting when dealing with passing arguments
    ./configure --prefix="${PREFIX:?}/moose" ${MOOSE_OPTIONS:-''} || return 1
    CORES="${MOOSE_JOBS:-2}"

    # moose_test-opt
    cd test
    make -j "${CORES:?}"
    make install -j "${CORES:?}"

    # combined-opt
    cd ../modules/combined
    make -j "${CORES:?}" || return 1
    make install -j "${CORES:?}" || return 1
}

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build

cd "${PREFIX:?}/moose/bin"
ln -s combined-opt moose-opt
ln -s combined-opt moose

# Fix (hack) for moose -> moose symlink collision binary/copy inputs
cd "${PREFIX:?}/moose/share/moose"
for f in ../combined/*; do
  [[ -e ${f} ]] || break  # handle the case of no *.wav files
  if [[ -d ../combined/${f} ]] && [[ ! -d ${f} ]] && [[ -f ${f} ]] && [[ ! -L ${f} ]]; then
    ln -s ../combined/"${f}" .
  fi
done

mkdir -p "${PREFIX:?}/etc/conda/activate.d" "${PREFIX:?}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export PATH=\${PATH}:${PREFIX}/moose/bin
export MOOSE_BIN=${PREFIX}/moose/bin/moose
export MOOSE_ADFPARSER_JIT_INCLUDE=${PREFIX}/moose/include/moose/ADRealMonolithic.h
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
export PATH=\${PATH%":${PREFIX}/moose/bin"}
unset MOOSE_BIN
unset MOOSE_ADFPARSER_JIT_INCLUDE
EOF
