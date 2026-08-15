#!/usr/bin/env bash
# Configure and build MOOSE with CMake, one build tree per METHOD -- the CMake analogue of the
# per-METHOD loop in docker_ci/build_moose.sh. Coexists with the make build; requires the
# moose-dev conda env (or LIBMESH_DIR/PETSC_DIR/WASP_DIR set, libmesh-config on PATH).
#
# Env:
#   MOOSE_METHODS  space-separated methods to build (default: "opt")
#   MOOSE_JOBS     parallel build jobs (default: detected)
#   MOOSE_CMAKE_TARGET  build only this target instead of the default (e.g. "moose-opt")
# Extra args are passed through to `cmake` configure (e.g. -DMOOSE_ENABLE_MODULES=OFF).
set -eu

MOOSE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MOOSE_METHODS="${MOOSE_METHODS:-opt}"
if [ -z "${MOOSE_JOBS:-}" ]; then
  MOOSE_JOBS="$( (command -v nproc >/dev/null && nproc) || sysctl -n hw.ncpu || echo 4)"
fi

generator="Unix Makefiles"
command -v ninja >/dev/null && generator="Ninja"

for method in $MOOSE_METHODS; do
  build_dir="${MOOSE_DIR}/build/${method}"
  echo "=== Configuring MOOSE (METHOD=${method}) in ${build_dir} ==="
  cmake -S "${MOOSE_DIR}" -B "${build_dir}" -G "${generator}" -DMOOSE_METHOD="${method}" "$@"
  echo "=== Building MOOSE (METHOD=${method}, -j${MOOSE_JOBS}) ==="
  if [ -n "${MOOSE_CMAKE_TARGET:-}" ]; then
    cmake --build "${build_dir}" -j "${MOOSE_JOBS}" --target "${MOOSE_CMAKE_TARGET}"
  else
    cmake --build "${build_dir}" -j "${MOOSE_JOBS}"
  fi
done
