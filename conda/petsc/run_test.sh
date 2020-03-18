#!/bin/bash
set -e

export PETSC_DIR=${PREFIX}

pkg-config --cflags PETSc | grep -v isystem
