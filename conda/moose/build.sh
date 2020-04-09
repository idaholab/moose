#!/bin/bash
set -eu

export LIBMESH_DIR=${PREFIX}/libmesh

cd framework
./configure --prefix=${PREFIX}/moose
cd ../test
make -j $CPU_COUNT
make install
cd ${PREFIX}/bin
ln -s ${PREFIX}/moose/moose_test-opt .
