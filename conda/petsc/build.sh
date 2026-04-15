#!/bin/bash
set -eu

sedinplace() {
  if [[ $(uname) == Darwin ]]; then
    sed -i "" "$@"
  else
    sed -i"" "$@"
  fi
}

function do_build(){
    export PETSC_DIR=$SRC_DIR
    export PETSC_ARCH=arch-conda-c-opt

    rm -rf "${SRC_DIR:?}/${PETSC_ARCH}"

    ## TODO: the following is a partial requirement for the day we introduce pytorch
    # Handle switches created by Conda variants
    # ADDITIONAL_ARGS=""
    #if [[ "${build_variant}" == 'cuda' ]]; then
    #  # hacky hack hack
    #  cd $BUILD_PREFIX/lib
    #  rm -f libnvToolsExt.so
    #  ln -s ../targets/x86_64-linux/lib/libnvToolsExt.so.1.0.0 libnvToolsExt.so
    #  cd -
    #
    #  CXXFLAGS+=" -I${PREFIX}/nsight-compute-2024.1.0/host/target-linux-x64/nvtx/include/nvtx3"
    #  CFLAGS+=" -I${PREFIX}/nsight-compute-2024.1.0/host/target-linux-x64/nvtx/include/nvtx3"
    #  ADDITIONAL_ARGS+=" --download-slate=1 --with-cuda=1 --with-cudac=${PREFIX}/bin/nvcc \
    #                     --with-cuda-dir=${PREFIX}/targets/x86_64-linux \
    #                     --CUDAFLAGS=-I${PREFIX}/targets/x86_64-linux/include"
    #fi
    # Now add ADDITIONAL_ARGS to the below configure_petsc arguments

    # set forth by MPI conda-forge package
    # shellcheck disable=SC2153
    HDF5_DIR=${PREFIX} configure_petsc \
        AR="${AR:-ar}" \
        CPP="$CPP" \
        RANLIB="$RANLIB" \
        CC="mpicc" \
        CXX="mpicxx" \
        FC="mpifort" \
        CPPFLAGS="$CPPFLAGS" \
        LDFLAGS="$LDFLAGS" \
        --COPTFLAGS="$CFLAGS -O3" \
        --CXXOPTFLAGS="$CXXFLAGS -O3" \
        --FOPTFLAGS="$FFLAGS -O3" \
        --with-clib-autodetect=0 \
        --with-cxxlib-autodetect=0 \
        --with-x=0 \
        --with-ssl=0 \
        --prefix="$PREFIX"

    make PETSC_DIR="$SRC_DIR" PETSC_ARCH=$PETSC_ARCH all
    make PETSC_DIR="$SRC_DIR" PETSC_ARCH=$PETSC_ARCH install

    # tired tired tired of broken, slow, Intel Macs
    # TODO: get testing working on macs
    if [[ $(uname) == 'linux' ]]; then
        # set forth by MPI conda-forge package
        # shellcheck disable=SC2154
        make SLEPC_DIR="$PREFIX" PETSC_DIR="$PREFIX" PETSC_ARCH="" check
    fi
}

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR}/configure_petsc.sh"

# unexport compiler variables to reduce warnings about config we know isn't used
# (This doesn't unset variables, just prevents the export for subprocesses)
export -n AR FC F90 F77 CC CXX CPP RANLIB
export -n CFLAGS CXXFLAGS CPPFLAGS FFLAGS LDFLAGS

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build


# Remove unneeded files
rm -f "$PREFIX"/lib/petsc/conf/configure-hash
find "$PREFIX"/lib/petsc -name '*.pyc' -delete

# remove abspath of executables in $BUILD_PREFIX
# let them resolve on $PATH
for f in $(grep -l "${BUILD_PREFIX}/bin/" -R "${PREFIX}/lib/petsc") "$PREFIX/lib/pkgconfig/PETSc.pc"; do
  echo "Fixing ${BUILD_PREFIX}/bin/ in $f"
  grep "${BUILD_PREFIX}/bin/" "$f" || true
  sedinplace s%"${BUILD_PREFIX}"/bin/%%g "$f"
done

# rewrite remaining $BUILD_PREFIX to $PREFIX
for f in $(grep -l "${BUILD_PREFIX}" -R "${PREFIX}/lib/petsc") "$PREFIX/lib/pkgconfig/PETSc.pc"; do
  echo "Fixing ${BUILD_PREFIX} in $f"
  grep "${BUILD_PREFIX}" "$f" || true
  sedinplace s%"${BUILD_PREFIX}"%"${PREFIX}"%g "$f"
done

# Strip GCC runtime libs on mac
if [[ $(uname) == Darwin ]]; then
  echo "Stripping GCC runtime libs"
  sed -i '' 's/-lemutls_w//g; s/-lheapt_w//g; s/-lgcc_s\.1[^ ]*//g' "$PREFIX"/lib/pkgconfig/PETSc.pc "$PREFIX"/lib/petsc/conf/petscvariables
fi

echo "Removing example files"
du -hs "$PREFIX"/share/petsc/examples/src
rm -fr "$PREFIX"/share/petsc/examples/src
echo "Removing data files"
du -hs "$PREFIX"/share/petsc/datafiles/*
rm -fr "$PREFIX"/share/petsc/datafiles

# Set PETSC_DIR environment variable for those that need it
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export PETSC_DIR=$PREFIX
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset PETSC_DIR
EOF

## TODO: the following is a partial requirement for the day we introduce pytorch
## Cuda specific activation/deactivation variables (append to above created script)
#if [[ "${build_variant}" == 'cuda' ]] && [[ "$mpi" == "openmpi" ]]; then
#cat <<EOF >> "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
#export OMPI_MCA_opal_cuda_support=true
#EOF
#cat <<EOF >> "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
#unset OMPI_MCA_opal_cuda_support
#EOF
#fi
