#!/bin/bash
set -exu

test "${LIBMESH_DIR:?}" == "${PREFIX:?}"/moose-libmesh

"${LIBMESH_DIR:?}"/bin/output_libmesh_version-opt

cp -r "${LIBMESH_DIR:?}"/examples/fem_system/ex1 .
cd ex1
make
mpiexec -n 2 ./example-opt -d 2 "${LIBMESH_DIR:?}"/share/reference_elements/2D/one_quad.xda
cd ..
rm -rf ex1
