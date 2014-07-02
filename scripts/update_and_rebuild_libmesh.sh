#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export LIBMESH_DIR=$SCRIPT_DIR/../libmesh/installed
export METHODS=${METHODS:="opt oprof dbg"}

cd $SCRIPT_DIR/..

# Test for git repository
git_dir=`git rev-parse --show-cdup 2>/dev/null`
if [[ $? == 0 && "x$git_dir" == "x" ]]; then
  git submodule init
  git submodule update
  if [[ $? != 0 ]]; then
    echo "git submodule command failed, are your proxy settings correct?"
    exit 1
  fi
fi

cd $SCRIPT_DIR/../libmesh

rm -rf build installed
mkdir build
cd build

../configure --with-methods="${METHODS}" \
             --prefix=$LIBMESH_DIR \
             --enable-silent-rules \
             --enable-unique-id \
             --disable-warnings \
             --disable-cxx11 \
             --enable-openmp $*

# let LIBMESH_JOBS be either MOOSE_JOBS, or 1 if MOOSE_JOBS
# is not set (not using our package). Make will then build
# with either JOBS if set, or LIBMESH_JOBS.
LIBMESH_JOBS=${MOOSE_JOBS:-1}

make -j ${JOBS:-$LIBMESH_JOBS}
make install
