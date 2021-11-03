#!/bin/bash
set -eu
export PATH=/bin:$PATH

export FCFLAGS="$FFLAGS"
export CC=$(basename "$CC")
export CXX=$(basename "$CXX")
export FC=$(basename "$FC")

if [[ $HOST == arm64-apple-darwin20.0.0 ]]; then
    # use Conda-Forge's Arm64 config.guess and config.sub, see
    # https://conda-forge.org/blog/posts/2020-10-29-macos-arm64/
    list_config_to_patch=$(find ./ -name config.guess | sed -E 's/config.guess//')
    for config_folder in $list_config_to_patch; do
        echo "copying config to $config_folder ...\n"
        cp -v $BUILD_PREFIX/share/gnuconfig/config.* $config_folder
    done
    ./autogen.sh
fi

unset LDFLAGS CPPFLAGS CFLAGS CXXFLAGS FFLAGS FCFLAGS F90 F77
export LDFLAGS="-L$PREFIX/lib -Wl,-rpath,$PREFIX/lib"
export LIBRARY_PATH="$PREFIX/lib"

if [[ $(uname) == Darwin ]]; then
    if [[ $target_platform == osx-arm64 ]]; then
        TUNING="-I$PREFIX/include"
        OPTIONS="--disable-opencl --enable-cxx --enable-fortran --with-device=ch3"
        export pac_cv_f77_accepts_F=yes
        export pac_cv_f77_flibs_valid=unknown
        export pac_cv_f77_sizeof_double_precision=8
        export pac_cv_f77_sizeof_integer=4
        export pac_cv_f77_sizeof_real=4
        export pac_cv_fc_accepts_F90=yes
        export pac_cv_fc_and_f77=yes
        export pac_cv_fc_module_case=lower
        export pac_cv_fc_module_ext=mod
        export pac_cv_fc_module_incflag=-I
        export pac_cv_fc_module_outflag=-J
        export pac_cv_fort90_real8=yes
        export pac_cv_fort_integer16=yes
        export pac_cv_fort_integer1=yes
        export pac_cv_fort_integer2=yes
        export pac_cv_fort_integer4=yes
        export pac_cv_fort_integer8=yes
        export pac_cv_fort_real16=no
        export pac_cv_fort_real4=yes
        export pac_cv_fort_real8=yes
        export pac_cv_prog_f77_and_c_stdio_libs=none
        export pac_cv_prog_f77_exclaim_comments=yes
        export pac_cv_prog_f77_has_incdir=-I
        export pac_cv_prog_f77_library_dir_flag=-L
        export pac_cv_prog_f77_mismatched_args=yes
        export pac_cv_prog_f77_mismatched_args_parm=
        export pac_cv_prog_f77_name_mangle='lower uscore'
        export pac_cv_prog_fc_and_c_stdio_libs=none
        export pac_cv_prog_fc_int_kind_16=8
        export pac_cv_prog_fc_int_kind_8=4
        export pac_cv_prog_fc_works=yes
        export pac_MOD='mod'
    else
        TUNING="-march=core2 -mtune=haswell -I$PREFIX/include"
        OPTIONS=""
    fi
    SHARED="clang"
else
    SHARED="gcc"
    TUNING="-march=nocona -mtune=haswell -I$PREFIX/include"
    OPTIONS=""
fi

./configure --prefix="${PREFIX}" \
            --enable-shared \
            --enable-sharedlibs="${SHARED}" \
            --enable-fast=O2 \
            --enable-debuginfo \
            --enable-two-level-namespace \
            CC="${CC}" CXX="${CXX}" FC="${FC}" F77="${FC}" F90="" \
            CFLAGS="${TUNING}" CXXFLAGS="${TUNING}" FFLAGS="${TUNING}" LDFLAGS="${LDFLAGS}" \
            FCFLAGS="${TUNING}" F90FLAGS="" F77FLAGS="" \
            ${OPTIONS}

make -j"${CPU_COUNT:-1}"
make install

# Set PETSC_DIR environment variable for those that need it
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77 C_INCLUDE_PATH=${PREFIX}/include MOOSE_NO_CODESIGN=true MPIHOME=${PREFIX}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset CC CXX FC F90 F77 C_INCLUDE_PATH MOOSE_NO_CODESIGN MPIHOME
EOF
