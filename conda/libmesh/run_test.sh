#!/bin/bash
set -exu

test "${LIBMESH_DIR:?}" == "${PREFIX:?}"/moose-libmesh

"${LIBMESH_DIR:?}"/bin/output_libmesh_version-opt

cp -r "${LIBMESH_DIR:?}"/examples/fem_system/ex1 .
cd ex1
for METHOD in opt oprof devel dbg; do
    make METHOD="${METHOD}"
    mpiexec -n 2 ./example-"${METHOD}" -d 2 "${LIBMESH_DIR:?}"/share/reference_elements/2D/one_quad.xda
done
cd ..
rm -rf ex1
