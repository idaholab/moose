#!/bin/bash
set -eu
export PATH=/bin:$PATH
export HYDRA_LAUNCHER=fork
export CC=mpicc CXX=mpicxx
mkdir -p build
cd build
VTK_PREFIX=${PREFIX}/libmesh-vtk
cmake .. -G "Ninja" \
    -Wno-dev \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH:PATH=${VTK_PREFIX} \
    -DCMAKE_INSTALL_PREFIX:PATH=${VTK_PREFIX} \
    -DCMAKE_INSTALL_RPATH:PATH=${VTK_PREFIX}/lib \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DCMAKE_OSX_SYSROOT=${CONDA_BUILD_SYSROOT} \
    -DVTK_BUILD_DOCUMENTATION:BOOL=OFF \
    -DVTK_BUILD_TESTING:BOOL=OFF \
    -DVTK_BUILD_EXAMPLES:BOOL=OFF \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DVTK_USE_MPI:BOOL=ON \
    -DVTK_GROUP_ENABLE_Rendering:STRING=DONT_WANT \
    -DVTK_GROUP_ENABLE_Qt::STRING=NO \
    -DVTK_GROUP_ENABLE_Views:STRING=NO \
    -DVTK_GROUP_ENABLE_Web:STRING=NO 

ninja install -v -j ${MOOSE_JOBS:-2}

# Set VTK environment variables for those that need it
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export VTKLIB_DIR=${VTK_PREFIX}/lib
export VTKINCLUDE_DIR=${VTK_PREFIX}/include/vtk-${vtk_friendly_version}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset VTKLIB_DIR
unset VTKINCLUDE_DIR
EOF
