#!/bin/bash
set -eu
export PATH=/bin:$PATH
export HYDRA_LAUNCHER=fork
export CC=mpicc CXX=mpicxx
export VTK_PREFIX=${PREFIX}/libmesh-vtk

# set in meta.yaml env package
# shellcheck disable=SC2154
export VTK_VER=${vtk_friendly_version}

# Tired of failing on build events that can be fixed by an invalidation on Civet.
function build_vtk() {
  # Settings guide: https://docs.vtk.org/en/latest/build_instructions/build_settings.html
  cmake .. -G "Ninja" \
      -Wno-dev \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH:PATH="${VTK_PREFIX}" \
      -DCMAKE_INSTALL_PREFIX:PATH="${VTK_PREFIX}" \
      -DCMAKE_INSTALL_RPATH:PATH="${VTK_PREFIX}"/lib \
      -DCMAKE_OSX_SYSROOT="${CONDA_BUILD_SYSROOT}" \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DBUILD_SHARED_LIBS:BOOL=ON \
      -DVTK_INSTALL_SDK:BOOL=ON \
      -DVTK_ENABLE_REMOTE_MODULES:BOOL=OFF \
      -DVTK_FORBID_DOWNLOADS:BOOL=ON \
      -DVTK_BUILD_DOCUMENTATION:BOOL=OFF \
      -DVTK_BUILD_TESTING:BOOL=OFF \
      -DVTK_BUILD_EXAMPLES:BOOL=OFF \
      -DVTK_BUILD_ALL_MODULES:BOOL=OFF \
      -DVTK_GROUP_ENABLE_Rendering:STRING=DONT_WANT \
      -DVTK_GROUP_ENABLE_Qt:STRING=NO \
      -DVTK_GROUP_ENABLE_Views:STRING=NO \
      -DVTK_GROUP_ENABLE_Web:STRING=NO \
      -DVTK_GROUP_ENABLE_MPI:STRING=DONT_WANT \
      -DVTK_GROUP_ENABLE_Imaging:STRING=DONT_WANT \
      -DVTK_USE_MPI:BOOL=ON \
      -DVTK_MODULE_ENABLE_VTK_IOMPIParallel:STRING=YES \
      -DVTK_MODULE_ENABLE_VTK_IOParallelExodus:STRING=YES \
      -DVTK_MODULE_ENABLE_VTK_IOParallelNetCDF:STRING=YES \
      -DVTK_MODULE_ENABLE_VTK_IOXML:STRING=YES \
      -DVTK_MODULE_ENABLE_VTK_IOXMLParser:STRING=YES \
      -DVTK_MODULE_ENABLE_VTK_IOImage:STRING=YES

  ninja install -v -j "${MOOSE_JOBS:-2}"
}

function no_exit_failure(){
  set +e
  (
    set -o pipefail
    build_vtk 2>&1 | tee -a "${SRC_DIR}"/output.log
  )
}

# Handle retries for this one step so as to not need an entire 4 hour build target redo.
TRY_AGAIN_REASON='Library not loaded: @rpath/'
while true; do
  mkdir -p "${SRC_DIR}"/build
  cd "${SRC_DIR}"/build
  if no_exit_failure; then
    set -e
    break
  elif [[ $("< ${SRC_DIR}"/output.log | grep -c "${TRY_AGAIN_REASON}") -eq 0 ]]; then
    tail -600 "${SRC_DIR}"/output.log && exit 1
  elif [[ ${try_count} -gt 2 ]]; then
    tail -100 "${SRC_DIR}"/output.log
    printf "Exhausted retry attempts: %s\n" "${try_count}"
    exit 1
  fi
  (( try_count++ )) || true
  tail -100 "${SRC_DIR}"/output.log
  printf "\n\nLibrary not loaded Conda bug. YUCK. Trying again.\n\n"
  # Start anew, clean.
  rm -rf "${SRC_DIR}"/build
  true > "${SRC_DIR}"/output.log
done

# Set VTK environment variables for those that need it
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export VTKLIB_DIR=${VTK_PREFIX}/lib VTKINCLUDE_DIR=${VTK_PREFIX}/include/vtk-${VTK_VER} VTK_VERSION=${VTK_VER}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset VTKLIB_DIR VTKINCLUDE_DIR VTK_VERSION
EOF
