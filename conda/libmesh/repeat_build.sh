LIBMESH_DIR=${PREFIX}/libmesh \
  configure_libmesh --with-vtk-lib=${BUILD_PREFIX}/libmesh-vtk/lib \
                    --with-vtk-include=${BUILD_PREFIX}/libmesh-vtk/include/vtk-${VTK_VERSION} \
                    $*

CORES=${MOOSE_JOBS:-2}
make -j $CORES
make install
