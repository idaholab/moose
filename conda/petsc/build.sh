#!/bin/bash
set -eu
export PETSC_DIR=$SRC_DIR
export PETSC_ARCH=arch-conda-c-opt

# Remove std=C++17 from CXXFLAGS as we specify the C++ dialect for PETSc as C++17 in configure_petsc.
# Specifying both causes an error as of PETSc 3.17.
CXXFLAGS=${CXXFLAGS//-std=c++[0-9][0-9]}
# This linker argument leads to segmentation faults when testing MUMPS during PETSc make check. We
# are not the only ones who have had problems with this option. See
# https://gitlab.c3s.unito.it/enocera/nnpdf/-/issues/307
LDFLAGS=${LDFLAGS//-Wl,-dead_strip_dylibs}

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

# Tired of failing on build events that can be fixed by an invalidation on Civet.
function build_petsc() {
  # made available by contents of meta.yaml (source: path ../../scripts)
  # shellcheck disable=SC1091
  source "$SRC_DIR"/configure_petsc.sh

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

function no_exit_failure(){
  set +e
  (
    set -o pipefail
    build_petsc 2>&1 | tee -a output.log
  )
}

# Handle retries for this one step so as to not need an entire 4 hour build target redo.
TRY_AGAIN_REASON="Library not loaded"
while true; do
  if no_exit_failure; then
    set -e
    break
  elif [[ $(grep -c "${TRY_AGAIN_REASON}" output.log) -eq 0 ]]; then
    tail -600 output.log && exit 1
  elif [[ ${try_count} -gt 2 ]]; then
    tail -100 output.log
    printf "Exhausted retry attempts: %s\n" "${try_count}"
    exit 1
  fi
  (( try_count++ )) || true
  tail -100 output.log
  printf "\n\nLibrary not loaded Conda bug. YUCK. Trying again.\n"
  # Start anew, clean.
  rm -rf "${PREFIX}"/petsc "${SRC_DIR}"/arch-conda-c-opt
  true > output.log
done

# Remove unneeded files
rm -f "${PREFIX}"/petsc/lib/petsc/conf/configure-hash
find "${PREFIX}"/petsc/lib/petsc -name '*.pyc' -delete

sedinplace() {
  if [[ $(uname) == Darwin ]]; then
    sed -i "" "$@"
  else
    sed -i"" "$@"
  fi
}

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
