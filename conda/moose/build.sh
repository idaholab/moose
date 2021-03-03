#!/bin/bash
set -eu

export LIBMESH_DIR=${PREFIX}/libmesh
./configure --prefix=${PREFIX}/moose
cd test
make -j $CPU_COUNT
make install
cd ${PREFIX}/bin
ln -s ${PREFIX}/moose/bin/moose_test-opt .
