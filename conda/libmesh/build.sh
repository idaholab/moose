#!/bin/bash
set -eu
export PATH=/bin:$PATH

export PKG_CONFIG_PATH=$BUILD_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH
export PETSC_DIR=`pkg-config PETSc --variable=prefix`

if [ -z $PETSC_DIR ]; then
    printf "PETSC not found.\n"
    exit 1
fi

function sed_replace(){
    if [ `uname` = "Darwin" ]; then
        sed -i '' -e "s|${BUILD_PREFIX}|${PREFIX}|g" $PREFIX/libmesh/bin/libmesh-config
    else
        sed -i'' -e "s|${BUILD_PREFIX}|${PREFIX}|g" $PREFIX/libmesh/bin/libmesh-config

        # Fix hard paths to /usr/bin/ when most operating system want these tools in /bin
        sed -i'' -e "s|/usr/bin/sed|/bin/sed|g" $PREFIX/libmesh/contrib/bin/libtool
        sed -i'' -e "s|/usr/bin/grep|/bin/grep|g" $PREFIX/libmesh/contrib/bin/libtool
        sed -i'' -e "s|/usr/bin/dd|/bin/dd|g" $PREFIX/libmesh/contrib/bin/libtool
    fi
}

mkdir -p build; cd build

if [[ $(uname) == Darwin ]]; then
    if [[ $HOST == arm64-apple-darwin20.0.0 ]]; then
        CTUNING="-march=armv8.3-a -I$PREFIX/include"
        LIBRARY_PATH="$PREFIX/lib"
    else
        CTUNING="-march=core2 -mtune=haswell"
    fi
else
    CTUNING="-march=nocona -mtune=haswell"
fi

unset LIBMESH_DIR CFLAGS CPPFLAGS CXXFLAGS FFLAGS LIBS \
      LDFLAGS DEBUG_CPPFLAGS DEBUG_CFLAGS DEBUG_CXXFLAGS \
      FORTRANFLAGS DEBUG_FFLAGS DEBUG_FORTRANFLAGS
export F90=mpifort
export F77=mpifort
export FC=mpifort
export CC=mpicc
export CXX=mpicxx
export CFLAGS="${CTUNING}"
export CXXFLAGS="${CTUNING}"
if [[ $HOST == arm64-apple-darwin20.0.0 ]]; then
    LDFLAGS="-L$PREFIX/lib -Wl,-S,-rpath,$PREFIX/lib"
else
    export LDFLAGS="-Wl,-S"
fi

export HYDRA_LAUNCHER=fork

source $SRC_DIR/configure_libmesh.sh
export INSTALL_BINARY="${SRC_DIR}/build-aux/install-sh -C"

# Tired of failing on build events that can be fixed by an invalidation on Civet.
# Handle retries for this one step so as to not need an entire 4 hour build target redo.
TRY_AGAIN_REASON='Library not loaded: @rpath/'
try_count=0
while true; do
  set +e
  (
    set -o pipefail
    source ${SRC_DIR}/repeat_build.sh 2>&1 | tee -a ${SRC_DIR}/output.log
  )
  if [[ $? -eq 0 ]]; then
    break
  elif [[ $(cat ${SRC_DIR}/output.log | grep -c "${TRY_AGAIN_REASON}") -eq 0 ]]; then
    tail -600 ${SRC_DIR}/output.log
    exit 1
  elif [[ ${try_count} > 2 ]]; then
    tail -600 ${SRC_DIR}/output.log
    printf "Exhausted retry attempts: ${try_count}\n"
    exit 1
  fi
  let try_count+=1
  tail -600 output.log
  printf "\n\nLibrary not loaded Conda bug. YUCK. Trying again.\n"
  # Start anew, clean.
  rm -rf ${PREFIX}/libmesh
  > ${SRC_DIR}/output.log
done
set -e

sed_replace

# Set LIBMESH_DIR, Eigen3_DIR
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export LIBMESH_DIR=${PREFIX}/libmesh
export Eigen3_DIR=${PREFIX}/libmesh/include/Eigen
EOF
# Unset previously set variables
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset LIBMESH_DIR
unset Eigen3_DIR
EOF
