#!/bin/bash
set -eux

# Install get_mac_sdk.sh script for getting the SDK on Macs
GET_MAC_SDK_PATH="share/moose-compilers/scripts/get_mac_sdk.sh"
if [[ "$(uname)" == "Darwin" ]]; then
    mkdir -p "${PREFIX}/$(dirname $GET_MAC_SDK_PATH)"
    cp "${SRC_DIR}/$(basename "$GET_MAC_SDK_PATH")" "${PREFIX}/${GET_MAC_SDK_PATH}"
fi

mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"

# Build compiler activation script
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_zzz_${PKG_NAME}.sh"
#!/bin/bash

# support ZSH initializations
if setopt &>/dev/null; then
    setopt local_options BASH_REMATCH
    setopt rematchpcre
fi

# flags that were set during the build process
# trailing spaces are intentional (see rematch regex below)
b_CXXFLAGS="${CXXFLAGS} "
b_CPPFLAGS="${CPPFLAGS} "
b_CFLAGS="${CFLAGS} "
b_FFLAGS="${FFLAGS} "
b_LDFLAGS="${LDFLAGS} "

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

export CPPFLAGS CXXFLAGS CFLAGS FFLAGS LDFLAGS
export C_INCLUDE_PATH=${PREFIX}/include
export CC=${PREFIX}/bin/mpicc
export CXX=${PREFIX}/bin/mpicxx
export FC=${PREFIX}/bin/mpif90
export F90=${PREFIX}/bin/mpif90
export F77=${PREFIX}/bin/mpif77
export HDF5_DIR=${PREFIX}
export MOOSE_NO_CODESIGN=true
export MPIHOME=${PREFIX}

EOF

# Special activation for macs
if [[ "$(uname)" == 'Darwin' ]]; then
    cat <<EOF >> "${PREFIX}/etc/conda/activate.d/activate_zzz_${PKG_NAME}.sh"

# Specific OS linker flags
LDFLAGS+=" -Wl,-ld_classic -Wl,-commons,use_dylibs"
export LDFLAGS

# Supported Mac SDK versions
SDK_VERSIONS=("26.2" "15.5" "15.4" "15.2" "14.5" "14.2" "13.3")

# Search for SDK
SDKROOT=
# Check for system SDK first
XCRUN_SDK_VERSION=
if XCRUN_SDK_VERSION="\$(xcrun --show-sdk-version 2> /dev/null)"; then
    for SDK_VERSION in "\${SDK_VERSIONS[@]}"; do
        if [[ "\$XCRUN_SDK_VERSION" == "\$SDK_VERSION" ]]; then
            SDKROOT="\$(xcrun --sdk macosx --show-sdk-path)" || break
        fi
    done
fi
# System SDK not available or is a bad version
if [ -z "\$SDKROOT" ]; then
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
fi
if [ -z "\$SDKROOT" ]; then
    use_color=
    [ -z "\$TERM" ] || [ "\$TERM" == "dumb" ] || [ -n "\$NO_COLOR" ] || use_color=1
    if [ -n "\$use_color" ]; then
        printf "\e[31m" >&2
    fi
    echo "ERROR: Mac SDK not found!" >&2
    echo "" >&2
    echo "This environment will not work properly without the SDK downloaded." >&2
    echo "" >&2
    echo "Run the following script to obtain it:" >&2
    echo "" >&2
    echo "${PREFIX}/${GET_MAC_SDK_PATH}" >&2
    if [ -n "\$use_color" ]; then
        printf "\e[0m" >&2
    fi
fi
export SDKROOT

EOF
fi

cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_zzz_${PKG_NAME}.sh"
unset CC CXX FC F90 F77 MOOSE_NO_CODESIGN MPIHOME HDF5_DIR
unset C_INCLUDE_PATH CPPFLAGS CXXFLAGS CFLAGS FFLAGS LDFLAGS
EOF

# shellcheck disable=SC2154
if [[ "${mpi}" == 'mpich' ]]; then
    cat <<EOF >> "${PREFIX}/etc/conda/activate.d/activate_zzz_${PKG_NAME}.sh"

# MPICH options
export FI_PROVIDER=tcp
export MPICH_CH4_NETMOD=ofi
EOF
    cat <<EOF >> "${PREFIX}/etc/conda/deactivate.d/deactivate_zzz_${PKG_NAME}.sh"

# Unset MPICH options
unset FI_PROVIDER
unset MPICH_CH4_NETMOD
EOF
elif [[ "${mpi}" == 'openmpi' ]]; then
    cat <<EOF >> "${PREFIX}/etc/conda/activate.d/activate_zzz_${PKG_NAME}.sh"

# OpenMPI options
export OMPI_MCA_mca_base_component_show_load_errors=0
EOF
    cat <<EOF >> "${PREFIX}/etc/conda/deactivate.d/deactivate_zzz_${PKG_NAME}.sh"
# Unset OpenMPI options
unset OMPI_MCA_mca_base_component_show_load_errors
EOF
fi
