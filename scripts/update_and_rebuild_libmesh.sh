#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

DIAGNOSTIC_LOG="libmesh_diagnostic.log"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Set go_fast flag if "--fast" is found in command line args.
for i in "$@"
do
  shift
  if [ "$i" == "--fast" ]; then
    go_fast=1;
  fi

  if [[ "$i" == "-h" || "$i" == "--help" ]]; then
    help=1;
  fi

  if [ "$i" == "--skip-submodule-update" ]; then
    skip_sub_update=1;
  elif [ "$i" == "--quiet-build" ]; then
    quiet_build=1;
    quiet_build_logfile="$SCRIPT_DIR/libmesh_build_$(date +%Y-%m-%d.%H:%M:%S).log"
  else # Remove everything else before passing to configure
    set -- "$@" "$i"
  fi
done

# Display help
if [[ -n "$help" ]]; then
  cd $SCRIPT_DIR/..
  echo "Usage: $0 [-h | --help | --fast | --skip-submodule-update | --quiet-build | <libmesh options> ]"
  echo
  echo "-h | --help              Display this message and list of available libmesh options"
  echo "--fast                   Run libmesh 'make && make install' only, do NOT run configure"
  echo "--skip-submodule-update  Do not update the libMesh submodule, use the current version"
  echo "--quiet-build            Only output the build (make and make install) to screen on failure"
  echo "*************************************************************************************"
  echo ""

  if [ -e "./libmesh/configure" ]; then
    libmesh/configure -h
  fi
  exit 0
fi

# If --fast was used, it means we are going to skip configure, so
# don't allow the user to pass any other flags (with the exception
# of --skip-submodule-update) to the script thinking they are going
# to do something.
if [[ -n "$go_fast" && $# != 1 ]]; then
  echo "Error: --fast can only be used by itself or with --skip-submodule-update."
  echo "Try again, removing either --fast or all other conflicting arguments!"
  exit 1;
fi


# generate machine diagnostics and write it to the log
$SCRIPT_DIR/diagnostics.sh > "$SCRIPT_DIR/$DIAGNOSTIC_LOG"

# Checks whether or not LIBMESH_DIR is a child of the conda base
function check_libmesh_dir_in_conda()
{
  type conda &> /dev/null || return
  CONDA_BASE="$(conda info --base)" 2> /dev/null || return
  LIBMESH_DIR_ABS="$(readlink -f $LIBMESH_DIR)" 2> /dev/null || return
  if [[ "$LIBMESH_DIR_ABS" == "$CONDA_BASE"* ]]; then
    echo "ERROR: LIBMESH_DIR=$LIBMESH_DIR exists within the conda base $CONDA_BASE"
    echo "Exiting in order to stop you from overwriting your conda environment"
    exit 1
  fi
}

if [[ -n "$LIBMESH_DIR" ]]; then
  check_libmesh_dir_in_conda

  echo "INFO: LIBMESH_DIR set - overriding default installed path"
  echo "INFO: No cleaning will be done in specified path"
  mkdir -p $LIBMESH_DIR
else
  export LIBMESH_DIR=$SCRIPT_DIR/../libmesh/installed
  rm -rf $SCRIPT_DIR/../libmesh/installed
fi

# If the user set METHOD, but not METHODS, we'll let METHOD override
# METHODS in this script. Otherwise, METHODS has a default in configure_libmesh.sh
if [[ -n "$METHOD" && -z "$METHODS" ]]; then
  export METHODS="$METHOD"
fi

# If the user has VTK available via our modules, we'll build with VTK
VTK_OPTIONS=""
if [[ -n "$VTKLIB_DIR" && -n "$VTKINCLUDE_DIR" ]]; then
  export VTK_OPTIONS="--with-vtk-lib=$VTKLIB_DIR --with-vtk-include=$VTKINCLUDE_DIR"
fi

cd $SCRIPT_DIR/..

# Test for git repository when not using fast
git_dir=`git rev-parse --show-cdup 2>/dev/null`
if [[ -z "$go_fast" && -z "$skip_sub_update" && $? == 0 && "x$git_dir" == "x" ]]; then
  git submodule update --init --recursive libmesh
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

# If PETSC_DIR is not set in the environment (perhaps because the user is not using the MOOSE
# package), we use the PETSc submodule
if [ -z "$PETSC_DIR" ]; then
  echo "PETSc submodule will be used. PETSc submodule is our default solver."
  echo "IMPORTANT: If you did not run the update_and_rebuild_petsc.sh script yet, please run it before building libMesh"
  export PETSC_DIR=$SCRIPT_DIR/../petsc
  if [ -z "$PETSC_ARCH" ]; then
    export PETSC_ARCH=arch-moose
  fi
fi

# If we're not going fast, remove the build directory and reconfigure
if [ -z "$go_fast" ]; then
  if [[ -n "$LIBMESH_BUILD_DIR" ]]; then
    echo "INFO: LIBMESH_BUILD_DIR set - overriding default build path"
  else
    export LIBMESH_BUILD_DIR=$SCRIPT_DIR/../libmesh/build
  fi
  rm -rf $LIBMESH_BUILD_DIR
  mkdir -p $LIBMESH_BUILD_DIR
  cd $LIBMESH_BUILD_DIR

  # The definition of INSTALL_BINARY, previously located here, is now located within the `configure_libmesh.sh`
  # script used below. This change was made to fixup a netCDF configure error related to supposed changes
  # in the environment from a previous run (even if the configure was the first performed). That was somehow
  # resolved by placing the INSTALL configure argument at the end of the configure line within the script. It
  # was determined that the INSTALL_BINARY definition should be placed within the function, to lessen confusion,
  # and that an explanation for longtime users be placed here for future reference. See #19230 for an example of
  # the error.

  # This is a temprorary fix, see #15120
  if [[ -n "$CPPFLAGS" ]]; then
    export CPPFLAGS=${CPPFLAGS//-DNDEBUG/}
    export CPPFLAGS=${CPPFLAGS//-O2/}
  fi
  if [[ -n "$CXXFLAGS" ]]; then
    export CXXFLAGS=${CXXFLAGS//-O2/}
  fi

  source $SCRIPT_DIR/configure_libmesh.sh
  SRC_DIR=${SCRIPT_DIR}/../libmesh configure_libmesh $DISABLE_TIMESTAMPS \
                                                     $VTK_OPTIONS \
                                                     $* | tee -a "$SCRIPT_DIR/$DIAGNOSTIC_LOG" || exit 1
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

if [[ -n "$quiet_build" ]]; then
  echo ""
  echo "Quiet build enabled (--quiet-build)"
  echo "Build output will be redirected to" $quiet_build_logfile "and will only be shown on failure"
  echo ""
  echo "Building quietly... this will take some time"
  echo ""
fi

# let LIBMESH_JOBS be either MOOSE_JOBS, or 1 if MOOSE_JOBS
# is not set (not using our package). Make will then build
# with either JOBS if set, or LIBMESH_JOBS.
LIBMESH_JOBS=${MOOSE_JOBS:-1}

# Helper for running build commands that allows us to redirect
# output to $quiet_build_logfile if --quiet-build is set
function run_build_cmd()
{
  echo "Running" $@"..."
  if [[ -n "$quiet_build" ]]; then
    $@ &> "$quiet_build_logfile"
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
      echo ""
      echo "Quiet build failed, printing output of "$quiet_build_logfile":"
      echo ""
      cat "$quiet_build_logfile"
      exit 1;
    fi
  else
    $@ || exit 1
  fi
}

if [ -z "${MOOSE_MAKE}" ]; then
  run_build_cmd make -j ${JOBS:-$LIBMESH_JOBS}
  run_build_cmd make install
else
  run_build_cmd ${MOOSE_MAKE}
  run_build_cmd ${MOOSE_MAKE} install
fi

if [[ -n "$quiet_build" ]]; then
  echo "Quiet build succeeded!"
fi

# Local Variables:
# sh-basic-offset: 2
# sh-indentation: 2
# End:
