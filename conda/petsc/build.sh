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

    rm -rf "${PREFIX:?}/petsc" "${SRC_DIR:?}/${PETSC_ARCH}"

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
    configure_petsc \
        --COPTFLAGS=-O3 \
        --CXXOPTFLAGS=-O3 \
        --FOPTFLAGS=-O3 \
        --with-x=0 \
        --with-ssl=0 \
        --with-mpi-dir="$PREFIX" \
        AR="$AR" \
        RANLIB="$RANLIB" \
        CFLAGS="$CFLAGS" \
        CXXFLAGS="$CXXFLAGS" \
        CPPFLAGS="$CPPFLAGS" \
        FFLAGS="$FFLAGS" \
        FCFLAGS="$FFLAGS" \
        LDFLAGS="$LDFLAGS" \
        --prefix="$PREFIX"/petsc

    make PETSC_DIR="$SRC_DIR" PETSC_ARCH=$PETSC_ARCH all
    make PETSC_DIR="$SRC_DIR" PETSC_ARCH=$PETSC_ARCH install

    # tired tired tired of broken, slow, Intel Macs
    if [[ "$(uname -m)" == 'arm64' ]] || [[ $(uname) == 'linux' ]]; then
        # set forth by MPI conda-forge package
        # shellcheck disable=SC2154
        make SLEPC_DIR="$PREFIX"/petsc PETSC_DIR="$PREFIX"/petsc PETSC_ARCH="" check
    fi
}

# Remove std=C++17 from CXXFLAGS as we specify the C++ dialect for PETSc as C++17 in
# configure_petsc. Specifying both causes an error as of PETSc 3.17.
CXXFLAGS=${CXXFLAGS//-std=c++[0-9][0-9]}

# This linker argument leads to segmentation faults when testing MUMPS during PETSc
# make check. We are not the only ones who have had problems with this option. See
# https://gitlab.c3s.unito.it/enocera/nnpdf/-/issues/307
LDFLAGS=${LDFLAGS//-Wl,-dead_strip_dylibs}

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR}/configure_petsc.sh"

# shellcheck disable=SC1091  # made available through meta.yaml src path
source "${SRC_DIR:?}/retry_build.sh"

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build

# Remove unneeded files
rm -f "${PREFIX}"/petsc/lib/petsc/conf/configure-hash
find "${PREFIX}"/petsc/lib/petsc -name '*.pyc' -delete

# Replace ${BUILD_PREFIX} after installation
grep -l "${BUILD_PREFIX}" -R "${PREFIX}/petsc/lib/petsc" | while IFS= read -r line; do
  echo "Fixing ${BUILD_PREFIX} in $line"
  sedinplace s%"${BUILD_PREFIX}"%"${PREFIX}"%g "$line"
done

echo "Removing example files"
du -hs "$PREFIX"/petsc/share/petsc/examples/src
rm -fr "$PREFIX"/petsc/share/petsc/examples/src
echo "Removing data files"
du -hs "$PREFIX"/petsc/share/petsc/datafiles/*
rm -fr "$PREFIX"/petsc/share/petsc/datafiles

# Set PETSC_DIR environment variable for those that need it
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export PETSC_DIR=${PREFIX}/petsc
export PKG_CONFIG_PATH=${PREFIX}/petsc/lib/pkgconfig:\${PKG_CONFIG_PATH}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset PETSC_DIR
export PKG_CONFIG_PATH=\${PKG_CONFIG_PATH%":${PREFIX}/petsc/lib/pkgconfig"}
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
