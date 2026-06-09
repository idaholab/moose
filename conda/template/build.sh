#!/bin/bash
set -eux

# shellcheck disable=SC1073
# shellcheck disable=SC1009
# shellcheck disable=SC1072  # these values are templated
export APPLICATION="<APPLICATION>"
export MOOSE_JOBS="<MOOSE_JOBS>"
export SKIP_DOCS="<SKIP_DOCS>"
export MOOSE_OPTIONS="<MOOSE_OPTIONS>"
export MOOSE_DOCS_FLAGS="<MOOSE_DOCS_FLAGS>"

# Keep a marker so we can act on only new files within the PREFIX
FILE_MARKER="${PREFIX:?}/file_marker"
touch "$FILE_MARKER"

function do_build(){
    # Perform cleanup in case we're trying again
    cd "${SRC_DIR:?}"
    git clean -xfd; git submodule foreach --recursive git clean -xfd

    # Expose python from MOOSE
    export PYTHONPATH=${SRC_DIR:?}<MOOSE>/python

    # Set variable for skipping documentation if applicable
    if [[ "${SKIP_DOCS}" == "True" ]]; then
        export MOOSE_SKIP_DOCS="True"
    fi

    # Configure MOOSE
    cd "${SRC_DIR:?}/moose"
    local -a moose_options
    read -ra moose_options <<< "$MOOSE_OPTIONS"
    ./configure --prefix=${PREFIX:?} "${moose_options[@]}"

    # Build and install application
    cd "${SRC_DIR:?}"
    make -j <MOOSE_JOBS> || return $?
    make install -j <MOOSE_JOBS> || return $?

    # Strip unneeded symbols from executables and libraries
    find "${PREFIX:?}/bin" -newer "$FILE_MARKER" -type f -print -exec strip -S {} \;
    if [[ "$(uname)" == "Darwin" ]]; then
        LIB_SUFFIX="dylib"
    else
        LIB_SUFFIX="so"
    fi
    find "${PREFIX:?}/lib" -newer "$FILE_MARKER" -type f -name "*.${LIB_SUFFIX}*" -print -exec strip -S {} \;
}

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

# Conda on mac sets "-Wl,-dead_strip_dylibs" in LDFLAGS, which is picked up
# during the application executable linking. Unfortunately, we end up with
# intermediate dead dylibs during our build/install (which we do fix later!).
# So... remove that flag on macs? Good, try conda.
if [[ "$(uname)" == "Darwin" ]]; then
  export LDFLAGS="${LDFLAGS//-Wl,-dead_strip_dylibs/}"
fi

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build

# Remove the file marker
rm "$FILE_MARKER"

mkdir -p "${PREFIX:?}/etc/conda/activate.d" "${PREFIX:?}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX:?}/etc/conda/activate.d/activate_${PKG_NAME:?}.sh"
export MOOSE_ADFPARSER_JIT_INCLUDE="${PREFIX}/include/moose/ADRealMonolithic.h"
export <FORMATTED_APPLICATION>_DOCS="${PREFIX}/share/<APPLICATION>/doc/index.html"
export NCRC_APP="<FORMATTED_APPLICATION>"
export NCRC_APP_RAW="<APPLICATION>"
EOF

cat <<EOF > "${PREFIX:?}/etc/conda/deactivate.d/deactivate_${PKG_NAME:?}.sh"
unset MOOSE_ADFPARSER_JIT_INCLUDE
unset <FORMATTED_APPLICATION>_DOCS
unset NCRC_APP
unset NCRC_APP_RAW
EOF

cat <<EOF >> "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export FI_PROVIDER=tcp
export MPICH_CH4_NETMOD=ofi
EOF

cat <<EOF >> "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset FI_PROVIDER
unset MPICH_CH4_NETMOD
EOF
