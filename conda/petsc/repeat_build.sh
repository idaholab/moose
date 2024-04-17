source $SRC_DIR/configure_petsc.sh
configure_petsc \
    --COPTFLAGS=-O3 \
    --CXXOPTFLAGS=-O3 \
    --FOPTFLAGS=-O3 \
    --with-x=0 \
    --with-ssl=0 \
    --with-mpi-dir=$PREFIX \
    AR="$AR" \
    RANLIB="$RANLIB" \
    CFLAGS="$CFLAGS" \
    CXXFLAGS="$CXXFLAGS" \
    CPPFLAGS="$CPPFLAGS" \
    FFLAGS="$FFLAGS" \
    FCFLAGS="$FFLAGS" \
    LDFLAGS="$LDFLAGS" \
    --download-mumps=$SRC_DIR/downloads/MUMPS_5.6.1.tar.gz \
    --download-ptscotch=$SRC_DIR/downloads/v7.0.4.tar.gz \
    --prefix=$PREFIX/petsc

make PETSC_DIR=$SRC_DIR PETSC_ARCH=$PETSC_ARCH all
make PETSC_DIR=$SRC_DIR PETSC_ARCH=$PETSC_ARCH install
if [[ "${mpi}" == 'mpich' ]]; then
    make PETSC_DIR=$SRC_DIR PETSC_ARCH=$PETSC_ARCH check
fi
