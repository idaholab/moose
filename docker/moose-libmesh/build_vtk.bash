#!/bin/bash
set -euxo pipefail

if [ -z "${VTK_VERSION:-}" ]; then
    echo "Build argument VTK_VERSION not set"
    exit 1
fi

# Source environment
set +u
source "${MOOSE_ENV:?}"
set -u

VTK_SHORT_VERSION="$(echo "$VTK_VERSION" | awk -F. '{print $1 "." $2}')"

# Obtain VTK
cd "${BUILD_DIR:?}"
VTK_TAR="VTK-${VTK_VERSION}.tar.gz"
VTK_URL="https://www.vtk.org/files/release/${VTK_SHORT_VERSION}/${VTK_TAR}"
curl -L -O "$VTK_URL"
tar -xf "$VTK_TAR"
rm -rf "$VTK_TAR"

# Build VTK
VTK_SRC_DIR="${BUILD_DIR}/$(basename "$VTK_TAR" .tar.gz)"
cd "$VTK_SRC_DIR"
mkdir build
cd build
cmake .. \
    -Wno-dev \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH:PATH="${VTK_DIR:?}" \
    -DCMAKE_INSTALL_PREFIX:PATH="${VTK_DIR:?}" \
    -DCMAKE_INSTALL_RPATH:PATH="${VTK_DIR:?}/lib" \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DVTK_BUILD_DOCUMENTATION:BOOL=OFF \
    -DVTK_BUILD_TESTING:BOOL=OFF \
    -DVTK_BUILD_EXAMPLES:BOOL=OFF \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DVTK_USE_MPI:BOOL=ON \
    -DVTK_USE_CUDA:BOOL=OFF \
    -DVTK_GROUP_ENABLE_Rendering:STRING=DONT_WANT \
    -DVTK_GROUP_ENABLE_Qt::STRING=NO \
    -DVTK_GROUP_ENABLE_Views:STRING=NO \
    -DVTK_GROUP_ENABLE_Web:STRING=NO
make install -j "${BUILD_JOBS:-4}"
rm -rf "$VTK_SRC_DIR"

# Add to environment
VTKINCLUDE_DIR="${VTK_DIR:?}/include/vtk-${VTK_SHORT_VERSION}"
echo "export VTK_DIR=${VTK_DIR:?}" >> "$MOOSE_ENV"
echo "export VTKINCLUDE_DIR=${VTKINCLUDE_DIR}" >> "$MOOSE_ENV"
