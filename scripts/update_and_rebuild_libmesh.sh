#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ ! -z "$LIBMESH_DIR" ]; then
  echo "INFO: LIBMESH_DIR set - overriding default installed path"
  echo "INFO: No cleaning will be done in specified path"
  mkdir -p $LIBMESH_DIR
else
  export LIBMESH_DIR=$SCRIPT_DIR/../libmesh/installed
  cd $SCRIPT_DIR/../libmesh
  rm -rf installed
  cd - >/dev/null # Make this quiet
fi

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

# check if the user has ccache configured
DISABLE_TIMESTAMPS=""
echo $CXX | cut -d ' ' -f1 | grep '^ccache$' > /dev/null
if [ $? == 0 ]; then
  echo -n "ccache detected - "
  # check if timestamps are explicitly enabled
  echo "$* " | grep -- '--enable-timestamps ' > /dev/null
  if [ $? == 0 ]; then
    echo "warning: setting --enable-timestamps explicitly will negatively impact the ccache performance"
  else
    echo "configuring libmesh with --disable-timestamps to improve cache hit rate"
    DISABLE_TIMESTAMPS="--disable-timestamps"
  fi
fi

cd $SCRIPT_DIR/../libmesh

rm -rf build
mkdir build
cd build

../configure --with-methods="${METHODS}" \
             --prefix=$LIBMESH_DIR \
             --enable-silent-rules \
             --enable-unique-id \
             --disable-warnings \
             --disable-cxx11 \
             --enable-unique-ptr \
             --enable-openmp \
             --disable-maintainer-mode \
             $DISABLE_TIMESTAMPS $*

# let LIBMESH_JOBS be either MOOSE_JOBS, or 1 if MOOSE_JOBS
# is not set (not using our package). Make will then build
# with either JOBS if set, or LIBMESH_JOBS.
LIBMESH_JOBS=${MOOSE_JOBS:-1}

if [ -z "${MOOSE_MAKE}" ]; then
  make -j ${JOBS:-$LIBMESH_JOBS} && \
    make install
else
  ${MOOSE_MAKE} && \
    ${MOOSE_MAKE} install
fi
