#!/bin/bash
set -eu
export PATH=/bin:$PATH

export PETSC_DIR=$SRC_DIR
export PETSC_ARCH=arch-conda-c-opt

export CC=$(basename "$CC")
export CXX=$(basename "$CXX")
export FC=$(basename "$FC")

# feed-stock recommendation
# scrub debug-prefix-map args, which cause problems in pkg-config
export CFLAGS=$(echo ${CFLAGS:-} | sed -E 's@\-fdebug\-prefix\-map[^ ]*@@g')
export CXXFLAGS=$(echo ${CXXFLAGS:-} | sed -E 's@\-fdebug\-prefix\-map[^ ]*@@g')
export FFLAGS=$(echo ${FFLAGS:-} | sed -E 's@\-fdebug\-prefix\-map[^ ]*@@g')
export FCFLAGS="$FFLAGS"
export HYDRA_LAUNCHER=fork

if [[ $(uname) == Darwin ]]; then
    if [[ $HOST == arm64-apple-darwin20.0.0 ]]; then
        CFLAGS="${CFLAGS} -mcpu=apple-a12"
        CXXFLAGS="${CXXFLAGS} -mcpu=apple-a12"
        FFLAGS="${FFLAGS} -march=armv8.3-a"
        FCFLAGS="${FCFLAGS} -march=armv8.3-a"
    else
        CFLAGS="${CFLAGS} -march=core2 -mtune=haswell"
        CXXFLAGS="${CXXFLAGS} -march=core2 -mtune=haswell"
        FFLAGS="${FFLAGS} -I$PREFIX/include"
        FCFLAGS="${FCFLAGS} -I$PREFIX/include"
    fi
else
    CFLAGS="${CFLAGS} -march=nocona -mtune=haswell"
    CXXFLAGS="${CXXFLAGS} -march=nocona -mtune=haswell"
    FFLAGS="${FFLAGS} -I$PREFIX/include"
    FCFLAGS="${FCFLAGS} -I$PREFIX/include"
fi

source $PETSC_DIR/configure_petsc.sh
configure_petsc \
    --COPTFLAGS=-O3 \
    --CXXOPTFLAGS=-O3 \
    --FOPTFLAGS=-O3 \
    --with-x=0 \
    --with-ssl=0 \
    CC="$CC" \
    CXX="$CXX" \
    FC="$FC" \
    F90="$F90" \
    F77="$F77" \
    AR="$AR" \
    RANLIB="$RANLIB" \
    CFLAGS="$CFLAGS" \
    CXXFLAGS="$CXXFLAGS" \
    CPPFLAGS="$CPPFLAGS" \
    FFLAGS="$FFLAGS" \
    FCFLAGS="$FCFLAGS" \
    LDFLAGS="$LDFLAGS" \
    --prefix=$PREFIX || (cat configure.log && exit 1)

# Verify that gcc_ext isn't linked
for f in $PETSC_ARCH/lib/petsc/conf/petscvariables $PETSC_ARCH/lib/pkgconfig/PETSc.pc; do
  if grep gcc_ext $f; then
    echo "gcc_ext found in $f"
    exit 1
  fi
done

sedinplace() {
  if [[ $(uname) == Darwin ]]; then
    sed -i "" "$@"
  else
    sed -i"" "$@"
  fi
}

# Remove abspath of ${BUILD_PREFIX}/bin/python
sedinplace "s%${BUILD_PREFIX}/bin/python%python%g" $PETSC_ARCH/include/petscconf.h
sedinplace "s%${BUILD_PREFIX}/bin/python%python%g" $PETSC_ARCH/lib/petsc/conf/petscvariables
sedinplace "s%${BUILD_PREFIX}/bin/python%/usr/bin/env python%g" $PETSC_ARCH/lib/petsc/conf/reconfigure-arch-conda-c-opt.py

# Replace abspath of ${PETSC_DIR} and ${BUILD_PREFIX} with ${PREFIX}
for path in $PETSC_DIR $BUILD_PREFIX; do
    for f in $(grep -l "${path}" $PETSC_ARCH/include/petsc*.h); do
        echo "Fixing ${path} in $f"
        sedinplace s%$path%\${PREFIX}%g $f
    done
done

make

# FIXME: Workaround mpiexec setting O_NONBLOCK in std{in|out|err}
# See https://github.com/conda-forge/conda-smithy/pull/337
# See https://github.com/pmodels/mpich/pull/2755
make check MPIEXEC="${RECIPE_DIR}/mpiexec.sh"

make install

# Remove unneeded files
rm -f ${PREFIX}/lib/petsc/conf/configure-hash
find $PREFIX/lib/petsc -name '*.pyc' -delete

# Replace ${BUILD_PREFIX} after installation,
# otherwise 'make install' above may fail
for f in $(grep -l "${BUILD_PREFIX}" -R "${PREFIX}/lib/petsc"); do
  echo "Fixing ${BUILD_PREFIX} in $f"
  sedinplace s%${BUILD_PREFIX}%${PREFIX}%g $f
done

echo "Removing example files"
du -hs $PREFIX/share/petsc/examples/src
rm -fr $PREFIX/share/petsc/examples/src
echo "Removing data files"
du -hs $PREFIX/share/petsc/datafiles/*
rm -fr $PREFIX/share/petsc/datafiles

# Set PETSC_DIR environment variable for those that need it
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export PETSC_DIR=${PREFIX}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset PETSC_DIR
EOF
