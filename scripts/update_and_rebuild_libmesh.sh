#!/usr/bin/env bash

# Set go_fast flag if "--fast" is found in command line args.
for i in "$@"
do
  if [ "$i" == "--fast" ]; then
    go_fast=1;
    break;
  fi
done

# If --fast was used, it means we are going to skip configure, so
# don't allow the user to pass any other flags to the script thinking
# they are going to do something.
if [[ -n "$go_fast" && $# != 1 ]]; then
  echo "Error: --fast cannot be used with other command line arguments to `basename "$0"`."
  echo "Try again, removing either --fast or all of the other arguments!"
  exit 1;
fi


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

# If the user sets both METHOD and METHODS, and they are not the same,
# this is an error.  Neither is treated as being more important than
# the other currently.
if [[ -n "$METHOD" && -n "$METHODS" ]]; then
  if [ "$METHOD" != "$METHODS" ]; then
    echo "Error: Both METHOD and METHODS are set, but are not equal."
    echo "Please set one or the other."
    exit 1
  fi
fi

# If the user set METHOD, we'll use that for METHODS in our script
if [ -n "$METHOD" ]; then
  export METHODS="$METHOD"
fi

# Finally, if METHODS is still not set, set a default value.
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

# If we're not going fast, remove the build directory and reconfigure
if [ -z "$go_fast" ]; then
  rm -rf build
  mkdir build
  cd build

  ../configure INSTALL="${SCRIPT_DIR}/../libmesh/build-aux/install-sh -C" \
               --with-methods="${METHODS}" \
               --prefix=$LIBMESH_DIR \
               --enable-silent-rules \
               --enable-unique-id \
               --disable-warnings \
               --enable-unique-ptr \
               --enable-openmp \
               --disable-maintainer-mode \
               $DISABLE_TIMESTAMPS $*
else
  # The build directory must already exist: you can't do --fast for
  # an initial build.
  if [ ! -d build ]; then
    echo "Error: You have no pre-existing build directory, so you can't use --fast."
    echo "Try running the script again without this flag."
    exit 1;
  fi

  cd build
fi

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

# Local Variables:
# sh-basic-offset: 2
# sh-indentation: 2
# End:
