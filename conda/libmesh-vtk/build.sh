#!/bin/bash
set -eu
export PATH=/bin:$PATH
export VTK_PREFIX="${PREFIX:?}/libmesh-vtk"
# shellcheck disable=SC2154  # set in meta.yaml env package
export VTK_VER=${vtk_friendly_version}

function do_build(){
    export HYDRA_LAUNCHER=fork
    export CC=mpicc CXX=mpicxx
    rm -rf "${VTK_PREFIX:?}" "${SRC_DIR:?}/build"
    mkdir -p "${SRC_DIR:?}/build"; cd "${SRC_DIR:?}/build"

    # Settings guide: https://docs.vtk.org/en/latest/build_instructions/build_settings.html
    cmake .. -G "Ninja" \
      -Wno-dev \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH:PATH="${VTK_PREFIX}" \
      -DCMAKE_INSTALL_PREFIX:PATH="${VTK_PREFIX}" \
      -DCMAKE_INSTALL_RPATH:PATH="${VTK_PREFIX}"/lib \
      -DCMAKE_OSX_SYSROOT="${CONDA_BUILD_SYSROOT:?}" \
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
      -DVTK_MODULE_ENABLE_VTK_IOImage:STRING=YES || return 1

    ninja install -v -j "${MOOSE_JOBS:-2}" || return 1
}

# shellcheck disable=SC1091  # made available through meta.yaml src path
source retry_build.sh

# Sets up retry functions and calls do_build. Blocking until success
# or 3 failed attempts, or 1 unknown/unhandled failure
retry_build

# Set VTK environment variables for those that need it
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export VTKLIB_DIR=${VTK_PREFIX}/lib VTKINCLUDE_DIR=${VTK_PREFIX}/include/vtk-${VTK_VER} VTK_VERSION=${VTK_VER}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset VTKLIB_DIR VTKINCLUDE_DIR VTK_VERSION
EOF
