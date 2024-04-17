#!/bin/bash
set -eu
export PATH=/bin:$PATH
export HYDRA_LAUNCHER=fork
export CC=mpicc CXX=mpicxx
export VTK_PREFIX=${PREFIX}/libmesh-vtk

# Tired of failing on build events that can be fixed by an invalidation on Civet.
# Handle retries for this one step so as to not need an entire 4 hour build target redo.
TRY_AGAIN_REASON='Library not loaded: @rpath/'
try_count=0
  while true; do
  mkdir -p ${SRC_DIR}/build
  cd ${SRC_DIR}/build
  set +e
  (
    set -o pipefail
    source ${SRC_DIR}/repeat_build.sh 2>&1 | tee -a ${SRC_DIR}/output.log
  )
  if [[ $? -eq 0 ]]; then
    break
  elif [[ $(cat ${SRC_DIR}/output.log | grep -c "${TRY_AGAIN_REASON}") -eq 0 ]]; then
    tail -600 ${SRC_DIR}/output.log
    exit 1
  elif [[ ${try_count} > 2 ]]; then
    tail -600 ${SRC_DIR}/output.log
    printf "Exhausted retry attempts: ${try_count}\n"
    exit 1
  fi
  let try_count+=1
  tail -600 ${SRC_DIR}/output.log
  printf "\n\nLibrary not loaded Conda bug. YUCK. Trying again.\n\n"
  # Start anew, clean.
  rm -rf ${SRC_DIR}/build
  > ${SRC_DIR}/output.log
done
set -e

# Set VTK environment variables for those that need it
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export VTKLIB_DIR=${VTK_PREFIX}/lib VTKINCLUDE_DIR=${VTK_PREFIX}/include/vtk-${vtk_friendly_version} VTK_VERSION=${vtk_friendly_version}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset VTKLIB_DIR VTKINCLUDE_DIR VTK_VERSION
EOF
