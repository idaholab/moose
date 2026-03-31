#!/bin/bash
set -eu

# Set MPI environment variables for those that need it, and set CXXFLAGS using our
# ACTIVATION_CXXFLAGS variable
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_zzz_${PKG_NAME}.sh"
#!/bin/bash

function baked_flags()
{
    # support ZSH initializations
    if setopt &>/dev/null; then
        setopt local_options BASH_REMATCH
        setopt rematchpcre
    fi

    # flags that were set during the build process
    # trailing spaces are intentional (see rematch regex below)
    local b_CXXFLAGS="${CXXFLAGS} "
    local b_CPPFLAGS="${CPPFLAGS} "
    local b_CFLAGS="${CFLAGS} "
    local b_FFLAGS="${FFLAGS} "
    local b_LDFLAGS="${LDFLAGS} "

    # flags we are always interested in removing
    o_flags=('-std=c++[0-9][0-9]')

    # flags we are interested in removing when not performing a conda build
    # if we are not building packages (conda build), strip off CPU tuning
    if [[ -z "\${CONDA_BUILD}" ]]; then
        if [[ \$b_FFLAGS =~ (-march=[a-zA-Z0-9.-]+) ]]; then
            o_flags+=("\${BASH_REMATCH[1]}")
        fi
        if [[ \$b_FFLAGS =~ (-mtune=[a-zA-Z0-9.-]+) ]]; then
            o_flags+=("\${BASH_REMATCH[1]}")
        fi
        # Lets assume this flag when used, is globbed together, separated by spaces
        if [[ \$b_CXXFLAGS =~ (-fdebug-prefix-map=.* ) ]]; then
            o_flags+=("\${BASH_REMATCH[1]}")
        fi

        # Offending flags we wish to remove
        o_flags+=('-D_FORTIFY_SOURCE=2' \
'-ftree-vectorize' \
'-DNDEBUG' \
'-O2' \
'-Wl,-dead_strip_dylibs' \
'-fno-stack-protector' \
'-ld_classic')
    fi

    # Remove any orphaned -Wl, flags (append last. Do not modify this line)
    o_flags+=('-Wl,[[:space:]]')

    for strip_flag in "\${o_flags[@]}"; do
        if [[ -z "\${strip_flag}" ]]; then continue; fi
        b_CXXFLAGS=\${b_CXXFLAGS//\${strip_flag}/}
        b_CPPFLAGS=\${b_CPPFLAGS//\${strip_flag}/}
        b_CFLAGS=\${b_CFLAGS//\${strip_flag}/}
        b_FFLAGS=\${b_FFLAGS//\${strip_flag}/}
        b_LDFLAGS=\${b_LDFLAGS//\${strip_flag}/}
    done

    # Remove orphaned -Wl, flags
    # Note we cannot use variable replacement here as it will not work with spaces in zsh
    b_CXXFLAGS="\$(echo "\$b_CXXFLAGS" | sed 's/-Wl,[[:space:]]//g')"
    b_CPPFLAGS="\$(echo "\$b_CPPFLAGS" | sed 's/-Wl,[[:space:]]//g')"
    b_CFLAGS="\$(echo "\$b_CFLAGS" | sed 's/-Wl,[[:space:]]//g')"
    b_FFLAGS="\$(echo "\$b_FFLAGS" | sed 's/-Wl,[[:space:]]//g')"
    b_LDFLAGS="\$(echo "\$b_LDFLAGS" | sed 's/-Wl,[[:space:]]//g')"

    # append necessary std c library
    CXXFLAGS="\${b_CXXFLAGS} -std=c++17"
    CPPFLAGS=\${b_CPPFLAGS}
    CFLAGS=\${b_CFLAGS}
    FFLAGS=\${b_FFLAGS}
    LDFLAGS=\${b_LDFLAGS}

    # special case for mac
    if [[ "\$(uname)" == 'Darwin' ]]; then
        # Specific OS linker flags
        LDFLAGS+=" -Wl,-ld64 -Wl,-commons,use_dylibs"

        # Make sure sdkroot is available
        SDK_VERSIONS=("14.5")
        SDK_SEARCH_DIRS=("/opt/" "/opt/conda-sdks" "/Users/\$(whoami)/sdks")
        SDKROOT=
        for SDK_VERSION in "\${SDK_VERSIONS[@]}"; do
            SDK_NAME="MacOSX\${SDK_VERSION}.sdk"
            for SDK_SEARCH_DIR in "\${SDK_SEARCH_DIRS[@]}"; do
                if [ -d "\${SDK_SEARCH_DIR}/\${SDK_NAME}" ]; then
                    SDKROOT="\${SDK_SEARCH_DIR}/\${SDK_NAME}"
                    break
                fi
            done
            if [ -n "\$SDKROOT" ]; then
                break
            fi
        done
        if [ -z "\$SDKROOT" ]; then
            echo "ERROR: Mac SDK not found!" >&2
            echo "" >&2
            echo "This environment will not work properly without the SDK downloaded." >&2
            echo "" >&2
            echo "Run ./scripts/extract_mac_sdk.sh in the MOOSE directory to obtain it." >&2
        fi

        if [ -z "\$CMAKE_ARGS" ]; then
            CMAKE_ARGS="-DCMAKE_OSX_SDKROOT=\${SDKROOT}"
        else
            CMAKE_ARGS+=" -DCMAKE_OSX_SDKROOT=\${SDKROOT}"
        fi
        export CMAKE_ARGS SDKROOT
    fi

    export CXXFLAGS CXXFLAGS CFLAGS FFLAGS LDFLAGS SDKROOT
}

export CC=mpicc \
CXX=mpicxx \
FC=mpif90 \
F90=mpif90 \
F77=mpif77 \
C_INCLUDE_PATH=${PREFIX}/include \
MOOSE_NO_CODESIGN=true \
MPIHOME=${PREFIX} \
HDF5_DIR=${PREFIX}

baked_flags
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_zzz_${PKG_NAME}.sh"
unset CC CXX FC F90 F77 MOOSE_NO_CODESIGN MPIHOME HDF5_DIR
unset C_INCLUDE_PATH CXXFLAGS CPPFLAGS CFLAGS FFLAGS LDFLAGS
EOF

# shellcheck disable=SC2154
if [[ "${mpi}" == 'mpich' ]]; then
    cat <<EOF >> "${PREFIX}/etc/conda/activate.d/activate_zzz_${PKG_NAME}.sh"
export FI_PROVIDER=tcp
export MPICH_CH4_NETMOD=ofi
EOF
    cat <<EOF >> "${PREFIX}/etc/conda/deactivate.d/deactivate_zzz_${PKG_NAME}.sh"
unset FI_PROVIDER
unset MPICH_CH4_NETMOD
EOF
elif [[ "${mpi}" == 'openmpi' ]]; then
    cat <<EOF >> "${PREFIX}/etc/conda/activate.d/activate_zzz_${PKG_NAME}.sh"
export OMPI_MCA_mca_base_component_show_load_errors=0
EOF
    cat <<EOF >> "${PREFIX}/etc/conda/deactivate.d/deactivate_zzz_${PKG_NAME}.sh"
unset OMPI_MCA_mca_base_component_show_load_errors
EOF
fi
