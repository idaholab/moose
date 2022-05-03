#!/bin/bash
set -eu
export PATH=/bin:$PATH

if [[ $mpi == "openmpi" ]]; then
  export OMPI_MCA_plm=isolated
  export OMPI_MCA_rmaps_base_oversubscribe=yes
  export OMPI_MCA_btl_vader_single_copy_mechanism=none
elif [[ $mpi == "moose-mpich" ]]; then
  export HYDRA_LAUNCHER=fork
fi

# Qt is enabled in VTK by default, but currently doesn't exist on M1 Macs.
# So, it is disabled if building on that platform.
QT="YES"
if [[ $(uname) == Darwin ]] && [[ $(uname -p) == arm ]]; then
  QT="NO"
fi

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
    -DVTK_BUILD_DOCUMENTATION:BOOL=OFF \
    -DVTK_BUILD_TESTING:BOOL=OFF \
    -DVTK_BUILD_EXAMPLES:BOOL=OFF \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DVTK_USE_MPI:BOOL=ON \
    -DVTK_WRAP_PYTHON:BOOL=ON \
    -DVTK_GROUP_ENABLE_Rendering:STRING=YES \
    -DVTK_GROUP_ENABLE_Qt::STRING=${QT} \
    -DVTK_GROUP_ENABLE_Views:STRING=NO \
    -DVTK_GROUP_ENABLE_Web:STRING=NO \
    -DCMAKE_OSX_SYSROOT=${CONDA_BUILD_SYSROOT}

ninja install -v -j ${MOOSE_JOBS:-2}

# Create a symlink between the VTK python module/deps and conda python
VTK_SP_DIR=${VTK_PREFIX}/lib/python3.*/site-packages
ln -s ${VTK_SP_DIR}/vtk.py ${SP_DIR}/vtk.py
ln -s ${VTK_SP_DIR}/vtkmodules ${SP_DIR}/vtkmodules
ln -s ${VTK_SP_DIR}/mpi4py ${SP_DIR}/mpi4py

# In case pkg_resources needs to be able to find VTK
cat > $SP_DIR/vtk-$PKG_VERSION.egg-info <<FAKE_EGG
Metadata-Version: 2.1
Name: vtk
Version: $PKG_VERSION
Summary: VTK is an open-source toolkit for 3D computer graphics, image processing, and visualization
Platform: UNKNOWN
FAKE_EGG

# Set VTK environment variables for those that need it
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export VTKLIB_DIR=${VTK_PREFIX}/lib
export VTKINCLUDE_DIR=${VTK_PREFIX}/include/vtk-${SHORT_VTK_NAME}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset VTKLIB_DIR
unset VTKINCLUDE_DIR
EOF
