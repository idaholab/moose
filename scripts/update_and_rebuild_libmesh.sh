#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo $SCRIPT_DIR

export LIBMESH_DIR=$SCRIPT_DIR/../libmesh/installed
export METHODS=${METHODS:="opt oprof dbg"}
export JOBS=${JOBS:=1}

cd $SCRIPT_DIR/..

git submodule init
git submodule update

cd $SCRIPT_DIR/../libmesh

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
