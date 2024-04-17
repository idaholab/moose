#!/bin/bash
set -eu
export PETSC_DIR=$SRC_DIR
export PETSC_ARCH=arch-conda-c-opt

# Remove std=C++17 from CXXFLAGS as we specify the C++ dialect for PETSc as C++17 in configure_petsc.
# Specifying both causes an error as of PETSc 3.17.
CXXFLAGS=${CXXFLAGS//-std=c++[0-9][0-9]}

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
#  ADDITIONAL_ARGS+=" --download-slate=1 --with-cuda=1 --with-cudac=${PREFIX}/bin/nvcc --with-cuda-dir=${PREFIX}/targets/x86_64-linux --CUDAFLAGS=-I${PREFIX}/targets/x86_64-linux/include"
#fi
# Now add ADDITIONAL_ARGS to the below configure_petsc arguments

# Manualy download these troublesome packages (due to our network)
mkdir downloads; cd downloads
curl -L -O https://web.cels.anl.gov/projects/petsc/download/externalpackages/MUMPS_5.6.1.tar.gz
curl -L -O https://github.com/live-clones/scotch/archive/refs/tags/v7.0.4.tar.gz
cd ../

# Tired of failing on build events that can be fixed by an invalidation on Civet.
# Handle retries for this one step so as to not need an entire 4 hour build target redo.
TRY_AGAIN_REASON="Library not loaded"
try_count=0
while true; do
  set +e
  (
    set -o pipefail
    source repeat_build.sh 2>&1 | tee -a output.log
  )
  if [[ $? -eq 0 ]]; then
    break
  elif [[ $(cat output.log | grep -c "${TRY_AGAIN_REASON}") -eq 0 ]]; then
    tail -600 output.log
    exit 1
  elif [[ ${try_count} > 2 ]]; then
    tail -600 output.log
    printf "Exhausted retry attempts: ${try_count}\n"
    exit 1
  fi
  let try_count+=1
  tail -600 output.log
  printf "\n\nLibrary not loaded Conda bug. YUCK. Trying again.\n"
  # Start anew, clean.
  rm -rf ${PREFIX}/petsc ${SRC_DIR}/arch-conda-c-opt
  > output.log
done
set -e

# Remove unneeded files
rm -f ${PREFIX}/petsc/lib/petsc/conf/configure-hash
find ${PREFIX}/petsc/lib/petsc -name '*.pyc' -delete

sedinplace() {
  if [[ $(uname) == Darwin ]]; then
    sed -i "" "$@"
  else
    sed -i"" "$@"
  fi
}

# Replace ${BUILD_PREFIX} after installation
for f in $(grep -l "${BUILD_PREFIX}" -R "${PREFIX}/petsc/lib/petsc"); do
  echo "Fixing ${BUILD_PREFIX} in $f"
  sedinplace s%${BUILD_PREFIX}%${PREFIX}%g $f
done

echo "Removing example files"
du -hs $PREFIX/petsc/share/petsc/examples/src
rm -fr $PREFIX/petsc/share/petsc/examples/src
echo "Removing data files"
du -hs $PREFIX/petsc/share/petsc/datafiles/*
rm -fr $PREFIX/petsc/share/petsc/datafiles

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
