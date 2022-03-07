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

# In order to set a CXXFLAGS during moose-mpich activation with the proper C++
# standard (as clang and gcc from conda-forge still default to C++14), we extract
# what moose_cxx gives us here, before we unset it. The  "-fdebug-prefix-map"
# flags are also removed, as they are specific to the conda build process.
TEMP_CXXFLAGS=${CXXFLAGS%%-fdebug-prefix-map*}
# Finally, swap "-std=c++14" with "-std=c++17" to set our basic standard requirement.
ACTIVATION_CXXFLAGS=${TEMP_CXXFLAGS/-std=c++14/-std=c++17}

unset LDFLAGS CPPFLAGS CFLAGS CXXFLAGS FFLAGS FCFLAGS F90 F77
export LDFLAGS="-L$PREFIX/lib -Wl,-rpath,$PREFIX/lib"
export LIBRARY_PATH="$PREFIX/lib"

if [[ $(uname) == Darwin ]]; then
    if [[ $target_platform == osx-arm64 ]]; then
        CTUNING="-mcpu=apple-a12 -I$PREFIX/include"
        FTUNING="-march=armv8.3-a -I$PREFIX/include"
        OPTIONS="--disable-opencl --enable-cxx --enable-fortran"
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
        CTUNING="-march=core2 -mtune=haswell -I$PREFIX/include"
        FTUNING="-march=core2 -mtune=haswell -I$PREFIX/include"
        OPTIONS=""
    fi
    SHARED="clang"
else
    SHARED="gcc"
    CTUNING="-march=nocona -mtune=haswell -I$PREFIX/include"
    FTUNING="-march=nocona -mtune=haswell -I$PREFIX/include"
    OPTIONS=""
fi

./configure --prefix="${PREFIX}" \
            --enable-shared \
            --enable-sharedlibs="${SHARED}" \
            --enable-fast=O2 \
            --enable-debuginfo \
            --enable-two-level-namespace \
            --with-device=ch3 \
            CC="${CC}" CXX="${CXX}" FC="${FC}" F77="${FC}" F90="" \
            CFLAGS="${CTUNING}" CXXFLAGS="${CTUNING}" FFLAGS="${FTUNING}" LDFLAGS="${LDFLAGS}" \
            FCFLAGS="${FTUNING}" F90FLAGS="" F77FLAGS="" \
            ${OPTIONS}
make -j $(./cpu_count.sh)
make install

# Set MPICH environment variables for those that need it, and set CXXFLAGS using our ACTIVATION_CXXFLAGS variable
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77 C_INCLUDE_PATH=${PREFIX}/include MOOSE_NO_CODESIGN=true MPIHOME=${PREFIX} CXXFLAGS="$ACTIVATION_CXXFLAGS"
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset CC CXX FC F90 F77 C_INCLUDE_PATH MOOSE_NO_CODESIGN MPIHOME CXXFLAGS
EOF
