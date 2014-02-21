#!/bin/bash

export LIBMESH_DIR=${LIBMESH_DIR:=`pwd`/libmesh/installed}
export METHODS=${METHODS:="opt oprof dbg"}
export JOBS=${JOBS:=1}

git submodule init
git submodule update
cd libmesh
rm -rf build installed
mkdir build
cd build
../configure --with-methods="${METHODS}" \
             --prefix=$LIBMESH_DIR \
             --enable-silent-rules \
             --enable-unique-id \
             --disable-warnings \
             --enable-openmp $*

make -j $JOBS
make install
