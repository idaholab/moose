#!/bin/bash
set -exu

test "${PETSC_DIR:?}" == "${PREFIX:?}"/moose-petsc

pkg-config --validate PETSc
pkg-config --cflags PETSc | grep -v isystem
pkg-config --libs PETSc

cat "${PETSC_DIR:?}"/lib/petsc/conf/petscvariables

cd tests
make ex1
FI_PROVIDER=tcp make runex1 MPIEXEC=mpiexec
