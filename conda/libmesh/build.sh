#!/bin/bash
set -eu
export PATH=/bin:$PATH

export PKG_CONFIG_PATH=$BUILD_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH
PETSC_DIR="$(pkg-config PETSc --variable=prefix)"
export PETSC_DIR

function sed_replace(){
    if [ "$(uname)" = "Darwin" ]; then
        sed -i '' -e "s|${BUILD_PREFIX}|${PREFIX}|g" "$PREFIX"/libmesh/bin/libmesh-config
    else
        sed -i'' -e "s|${BUILD_PREFIX}|${PREFIX}|g" "$PREFIX"/libmesh/bin/libmesh-config

        # Fix hard paths to /usr/bin/ when most operating system want these tools in /bin
        sed -i'' -e "s|/usr/bin/sed|/bin/sed|g" "$PREFIX"/libmesh/contrib/bin/libtool
        sed -i'' -e "s|/usr/bin/grep|/bin/grep|g" "$PREFIX"/libmesh/contrib/bin/libtool
        sed -i'' -e "s|/usr/bin/dd|/bin/dd|g" "$PREFIX"/libmesh/contrib/bin/libtool
    fi
}

mkdir -p build; cd build

if [[ "$(uname)" == Darwin ]]; then
    if [[ $HOST == arm64-apple-darwin20.0.0 ]]; then
        CTUNING="-march=armv8.3-a -I$PREFIX/include"
        export LIBRARY_PATH="$PREFIX/lib"
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

# made available by contents of meta.yaml (source: path ../../scripts)
# shellcheck disable=SC1091
source "$SRC_DIR"/configure_libmesh.sh
export INSTALL_BINARY="${SRC_DIR}/build-aux/install-sh -C"

# Tired of failing on build events that can be fixed by an invalidation on Civet.
function build_libmesh() {
  export LIBMESH_DIR="${PREFIX}"/libmesh
  configure_libmesh --with-vtk-lib="${BUILD_PREFIX}"/libmesh-vtk/lib \
                    --with-vtk-include="${BUILD_PREFIX}"/libmesh-vtk/include/vtk-"${VTK_VERSION}"

  CORES=${MOOSE_JOBS:-6}
  make -j "$CORES"
  make install -j "$CORES"
}

function no_exit_failure(){
  set +e
  (
    set -o pipefail
    build_libmesh 2>&1 | tee -a "${SRC_DIR}"/output.log
  )
}

# Handle retries for this one step so as to not need an entire 4 hour build target redo.
TRY_AGAIN_REASON='Library not loaded: @rpath/'
while true; do
  if no_exit_failure; then
    set -e
    break
  elif [[ $(grep -c "${TRY_AGAIN_REASON}" "${SRC_DIR}"/output.log) -eq 0 ]]; then
    tail -600 "${SRC_DIR}"/output.log && exit 1
  elif [[ ${try_count} -gt 2 ]]; then
    tail -100 "${SRC_DIR}"/output.log
    printf "Exhausted retry attempts: %s\n" "${try_count}"
    exit 1
  fi
  (( try_count++ )) || true
  tail -100 output.log
  printf "\n\nLibrary not loaded Conda bug. YUCK. Trying again.\n"
  # Start anew, clean.
  rm -rf "${PREFIX}"/libmesh
  true > "${SRC_DIR}"/output.log
done

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
