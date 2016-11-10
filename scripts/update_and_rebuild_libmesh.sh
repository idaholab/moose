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

if [[ -n "$LIBMESH_DIR" ]]; then
  echo "INFO: LIBMESH_DIR set - overriding default installed path"
  echo "INFO: No cleaning will be done in specified path"
  mkdir -p $LIBMESH_DIR
else
  export LIBMESH_DIR=$SCRIPT_DIR/../libmesh/installed
  rm -rf $SCRIPT_DIR/../libmesh/installed
fi

# If the user set METHOD, but not METHODS, we'll let METHOD override
# METHODS in this script.
if [[ -n "$METHOD" && -z "$METHODS" ]]; then
  export METHODS="$METHOD"
fi

# If the user has VTK available via our modules, we'll build with VTK
VTK_OPTIONS=""
if [[ -n "$VTKLIB_DIR" && -n "$VTKINCLUDE_DIR" ]]; then
  export VTK_OPTIONS="--with-vtk-lib=$VTKLIB_DIR --with-vtk-include=$VTKINCLUDE_DIR"
fi

# Finally, if METHODS is still not set, set a default value.
export METHODS=${METHODS:="opt oprof dbg"}

cd $SCRIPT_DIR/..

# Test for git repository when not using fast
git_dir=`git rev-parse --show-cdup 2>/dev/null`
if [[ -z "$go_fast" && $? == 0 && "x$git_dir" == "x" ]]; then
  git submodule init
  git submodule update
  if [[ $? != 0 ]]; then
    echo "git submodule command failed, are your proxy settings correct?"
    # TODO: is this a git bug?
    # failed attempts with `git submodule update` deletes the submodule directory.
    # So re-create it to prevent a diff.
    mkdir libmesh
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
               --enable-petsc-required \
               $DISABLE_TIMESTAMPS $VTK_OPTIONS $* || exit 1
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
