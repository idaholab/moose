#!/bin/bash
set -eux
export PKG_CONFIG_PATH=$BUILD_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH
export PETSC_DIR=`pkg-config PETSc --variable=prefix`

if [ -z $PETSC_DIR ]; then
    printf "PETSC not found.\n"
    exit 1
fi

function sed_replace(){
    if [ `uname` = "Darwin" ]; then
        sed -i '' -e "s|${BUILD_PREFIX}|${PREFIX}|g" $PREFIX/libmesh/bin/libmesh-config
    else
        sed -i'' -e "s|${BUILD_PREFIX}|${PREFIX}|g" $PREFIX/libmesh/bin/libmesh-config

        # Fix hard paths to /usr/bin/ when most operating system want these tools in /bin
        sed -i'' -e "s|/usr/bin/sed|/bin/sed|g" $PREFIX/libmesh/contrib/bin/libtool
        sed -i'' -e "s|/usr/bin/grep|/bin/grep|g" $PREFIX/libmesh/contrib/bin/libtool
        sed -i'' -e "s|/usr/bin/dd|/bin/dd|g" $PREFIX/libmesh/contrib/bin/libtool
    fi
}

mkdir -p build; cd build

if [[ $(uname) == Darwin ]]; then
    TUNING="-march=core2 -mtune=haswell"
else
    TUNING="-march=nocona -mtune=haswell"
fi

unset LIBMESH_DIR CFLAGS CPPFLAGS CXXFLAGS FFLAGS LIBS \
      LDFLAGS DEBUG_CPPFLAGS DEBUG_CFLAGS DEBUG_CXXFLAGS \
      FORTRANFLAGS DEBUG_FFLAGS DEBUG_FORTRANFLAGS
export F90=mpifort
export F77=mpifort
export FC=mpifort
export CC=mpicc
export CXX=mpicxx
export CFLAGS="${TUNING}"
export CXXFLAGS="${TUNING}"
export LDFLAGS="-Wl,-S"

if [[ $mpi == "openmpi" ]]; then
  export OMPI_MCA_plm=isolated
  export OMPI_MCA_rmaps_base_oversubscribe=yes
  export OMPI_MCA_btl_vader_single_copy_mechanism=none
elif [[ $mpi == "moose-mpich" ]]; then
  export HYDRA_LAUNCHER=fork
fi

BUILD_CONFIG=`cat <<EOF
--enable-silent-rules \
--enable-unique-id \
--disable-warnings \
--enable-glibcxx-debugging \
--with-thread-model=openmp \
--disable-maintainer-mode \
--enable-petsc-hypre-required \
--enable-metaphysicl-required
EOF`

../configure ${BUILD_CONFIG} \
                     --prefix=${PREFIX}/libmesh \
                     --with-vtk-lib=${BUILD_PREFIX}/libmesh-vtk/lib \
                     --with-vtk-include=${BUILD_PREFIX}/libmesh-vtk/include/vtk-${SHORT_VTK_NAME} \
                     --with-methods="opt oprof devel dbg" \
                     --without-gdb-command

make -j $CPU_COUNT
make install
sed_replace

# Set LIBMESH_DIR environment variable for those that need it
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export LIBMESH_DIR=${PREFIX}/libmesh
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset LIBMESH_DIR
unset MOOSE_NO_CODESIGN
EOF
