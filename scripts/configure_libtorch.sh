#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

function get_variable()
{
  if [ -z "${!1}" ]; then
    >&2 echo "ERROR: Missing required variable $1"
  fi
  echo "${!1}"
}

function underlying_compiler() {
  local c="$1"

  if "$c" --showme:command >/dev/null 2>&1; then
    "$c" --showme:command
  elif "$c" -show >/dev/null 2>&1; then
    "$c" -show | awk '{print $1}'
  elif "$c" -compile-info >/dev/null 2>&1; then
    "$c" -compile-info | awk '{print $1}'
  else
    echo "$c"
  fi
}

# Set the build options for the PyTorch Python package
#
# PyTorch's setup.py forwards BUILD_*, USE_* and CMAKE_* environment variables to its cmake
# configure step, so these are the environment-variable spelling of the flags that
# configure_libtorch passes as -D arguments. They deliberately differ in the options that only
# make sense once the Python package is built, which the C++-only configuration turns off. These
# are set explicitly (rather than left to PyTorch's defaults) so that they also override the
# cached OFF values when the C++ build directory is reused:
#   * BUILD_PYTHON: this is the Python package
#   * BUILD_FUNCTORCH: NEML2 uses torch.func (see neml2/models/param_ad.py)
#   * USE_NUMPY: NEML2 uses NumPy interop (see neml2/texture/polefigure.py)
function set_libtorch_python_build_options()
{
  export CMAKE_C_COMPILER="$(underlying_compiler "${CC:-cc}")"
  export CMAKE_CXX_COMPILER="$(underlying_compiler "${CXX:-c++}")"
  export CMAKE_BUILD_TYPE=Release
  export CMAKE_INCLUDE_PATH="$(get_variable PETSC_DIR)/include"
  export CMAKE_LIBRARY_PATH="$(get_variable PETSC_DIR)/lib"

  export BUILD_BINARY=0 BUILD_FUNCTORCH=1 BUILD_LITE_INTERPRETER=0 BUILD_PYTHON=1 BUILD_TEST=0
  export USE_BLAS=1 BLAS=OpenBLAS USE_LAPACK=1
  export USE_DISTRIBUTED=0 USE_GLOO=0 USE_MPI=0 USE_NCCL=0 USE_TENSORPIPE=0 USE_XCCL=0
  export USE_FBGEMM=0 USE_ITT=0 USE_KINETO=0 USE_MKLDNN=0 USE_NNPACK=0
  export USE_NUMPY=1 USE_OBSERVERS=0 USE_PYTORCH_QNNPACK=0 USE_VALGRIND=0 USE_XNNPACK=0
}

# Configure libtorch with the default MOOSE configuration options
#
# Separated so that it can be used across all libtorch build scripts:
# - scripts/update_and_rebuild_libtorch.sh
#
# Arguments:
#   1. Path to the libtorch source directory
#   2. Path to the libtorch build directory
#   3. Path to the libtorch installation directory
#
# Remaining arguments will be appended to the cmake command verbatim
function configure_libtorch()
{
  ARGS=()
  if which ninja &> /dev/null; then
    ARGS+=("-GNinja")
  fi
  ARGS+=("${@:4}")

  # conda-forge clang (the default_cfg builds, from 20.1.8/21.1.7) reads a configuration file
  # next to the compiler that adds -isystem $CONDA_PREFIX/include ahead of every command-line
  # include directory. The environment's protobuf (pulled in by pyarrow) then shadows the protobuf
  # 3.13 that libtorch vendors and builds against. --no-default-config skips that file; an
  # activated environment already supplies the same include, -L and rpath flags through
  # CPPFLAGS/CXXFLAGS and LDFLAGS, after libtorch's own include directories. GCC rejects the flag,
  # so it is only added for a compiler that accepts it.
  local c_compiler cxx_compiler c_cfg="" cxx_cfg=""
  c_compiler="$(underlying_compiler "${CC:-cc}")"
  cxx_compiler="$(underlying_compiler "${CXX:-c++}")"
  "$c_compiler" --no-default-config -E -x c /dev/null &> /dev/null && c_cfg="--no-default-config"
  "$cxx_compiler" --no-default-config -E -x c++ /dev/null &> /dev/null && cxx_cfg="--no-default-config"

  # Documenting the flag choices:
  # -DCMAKE_INCLUDE_PATH and -DCMAKE_LIBRARY_PATH are needed to find the PETSc installation,
  #    which is needed to make sure BLAS and LAPACK are compatible with PETSc and vice versa.
  # -DBUILD_PYTHON=OFF, -DBUILD_TEST=OFF, -DBUILD_BINARY=OFF, and -DBUILD_LITE_INTERPRETER=OFF
  #    are needed to avoid building unnecessary targets that we don't need.
  # -DBUILD_FUNCTORCH=OFF, -DUSE_GLOO=OFF, -DUSE_NCCL=OFF, -DUSE_XCCL=OFF, and -DUSE_TENSORPIPE=OFF
  #    are needed to avoid building distributed training support, which we don't need.
  # -DUSE_MPI=OFF and -DUSE_DISTRIBUTED=OFF are needed to avoid building MPI support, which we don't need.
  # -DUSE_MAGMA=ON is needed to enable MAGMA support, which we want for GPU support on linear algebra operations on large matrices.
  # -DUSE_MKLDNN=OFF, -DUSE_NNPACK=OFF, -DUSE_XNNPACK=OFF, and -DUSE_PYTORCH_QNNPACK=OFF are
  #    needed to avoid building support for various CPU acceleration libraries that we don't need.
  # -DUSE_KINETO=OFF: we don't need Kineto because we don't use PyTorch's profiler
  # -DUSE_FBGEMM=OFF: we don't need FBGEMM because we don't use quantization
  # -DUSE_NUMPY=OFF: we don't need NumPy support because we don't use PyTorch's C++ API for Python bindings
  # -DUSE_ITT=OFF: we don't need ITT support because we don't use PyTorch's profiler
  # -DUSE_VALGRIND=OFF: we don't need Valgrind support because we don't use PyTorch's C++ API for Python bindings
  #    and we don't use PyTorch's profiler
  # -DUSE_OBSERVERS=OFF: we don't need observers support because we don't use PyTorch's profiler
  CFLAGS="${c_cfg} ${CFLAGS:-}" \
  CXXFLAGS="${cxx_cfg} ${CXXFLAGS:-}" \
  cmake \
    -DCMAKE_C_COMPILER="${c_compiler}" \
    -DCMAKE_CXX_COMPILER="${cxx_compiler}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$3" \
    -DBUILD_PYTHON=OFF \
    -DBUILD_TEST=OFF \
    -DBUILD_BINARY=OFF \
    -DBUILD_LITE_INTERPRETER=OFF \
    -DBUILD_FUNCTORCH=OFF \
    -DUSE_GLOO=OFF \
    -DUSE_NCCL=OFF \
    -DUSE_XCCL=OFF \
    -DUSE_TENSORPIPE=OFF \
    -DCMAKE_INCLUDE_PATH="$(get_variable PETSC_DIR)/include" \
    -DCMAKE_LIBRARY_PATH="$(get_variable PETSC_DIR)/lib" \
    -DUSE_BLAS=ON \
    -DBLAS=OpenBLAS \
    -DUSE_LAPACK=ON \
    -DUSE_MPI=OFF \
    -DUSE_DISTRIBUTED=OFF \
    -DUSE_MAGMA=ON \
    -DUSE_MKLDNN=OFF \
    -DUSE_NNPACK=OFF \
    -DUSE_XNNPACK=OFF \
    -DUSE_PYTORCH_QNNPACK=OFF \
    -DBUILD_TEST=OFF \
    -DUSE_KINETO=OFF \
    -DUSE_FBGEMM=OFF \
    -DUSE_NUMPY=OFF \
    -DUSE_ITT=OFF \
    -DUSE_VALGRIND=OFF \
    -DUSE_OBSERVERS=OFF \
    -B "$2" \
    -S "$1" \
    "${ARGS[@]}"
}

# Build libtorch assuming that the project has already been configured
#
# Arguments:
#   1. Path to the libtorch build directory
#   2. Number of jobs to use when building libtorch
function build_libtorch()
{
  cmake --build "$1" --parallel "$2" --target all
}

# Install libtorch
#
# Arguments:
#   1. Path to the libtorch build directory
#   2. Install prefix
function install_libtorch()
{
  cmake --install "$1" --prefix "$2"
}

# Verify that the Python build and runtime dependencies required to build and install
# PyTorch from source (pyproject.toml build-system.requires and project.dependencies,
# plus numpy for USE_NUMPY=1 and cmake on PATH) are satisfied in an environment.
#
# Arguments:
#   1. Python executable to check
function check_libtorch_python_dependencies()
{
  "$1" -c '
import importlib.metadata as md, shutil, sys
reqs = [
    "filelock",
    "fsspec>=0.8.5",
    "jinja2",
    "networkx>=2.5.1",
    "numpy",
    "packaging>=24.2",
    "pyyaml",
    "requests",
    "setuptools>=77.0.3",
    "sympy>=1.13.3",
    "typing-extensions>=4.10.0",
]
missing = []
if not shutil.which("cmake"):
    missing.append("cmake (executable on PATH)")
try:
    from packaging.requirements import Requirement
except Exception:
    missing.append("packaging>=24.2")
else:
    for r in reqs:
        req = Requirement(r)
        try:
            ver = md.version(req.name)
        except md.PackageNotFoundError:
            missing.append(r)
            continue
        if req.specifier and not req.specifier.contains(ver, prereleases=True):
            missing.append(f"{r} (found {ver})")
if missing:
    print("Missing or incompatible PyTorch dependencies: " + ", ".join(missing), file=sys.stderr)
    sys.exit(1)
'
}

# Install the PyTorch package into a Python environment.
#
# Arguments:
#   1. Path to the PyTorch source directory
#   2. Python executable into whose environment PyTorch will be installed
#   3. Number of build jobs
function install_libtorch_python()
(
  local source_dir="$1"
  local python_executable="$2"

  set_libtorch_python_build_options
  export MAX_JOBS="$3"

  # Detect which pip is available. In a standard Python environment (e.g. conda)
  # `python3 -m pip` works; in the uv-created apptainer venv there is no pip module,
  # so fall back to the `pip` wrapper that forwards to `uv pip`.
  if "$python_executable" -m pip --version &> /dev/null; then
    PIP_EXE=("$python_executable" -m pip)
  elif command -v pip &> /dev/null; then
    PIP_EXE=(pip)
  else
    echo "ERROR: pip not found"
    exit 1
  fi

  # When the environment already satisfies PyTorch's build and runtime dependencies
  # (e.g. the moose-dev conda environment in network-restricted CI jobs), skip installing
  # requirements.txt from PyPI so offline builds succeed and existing conda packages
  # (cmake, ninja, setuptools) are not replaced.
  if ! check_libtorch_python_dependencies "$python_executable" 2>/dev/null; then
    "${PIP_EXE[@]}" install --no-cache -r "$source_dir/requirements.txt" || return
    check_libtorch_python_dependencies "$python_executable" || return
  fi

  # PyTorch's setup.py always builds in <source>/build, which is also the build directory used by
  # the C++-only build. When that build directory already exists, reconfigure it in place and keep
  # its cache so the Python package reuses the objects already compiled there instead of rebuilding
  # from scratch. setup.py only translates the environment variables set above into cmake options
  # when it configures from scratch, so the options where the Python build disagrees with the cache
  # left behind by configure_libtorch (BUILD_PYTHON, BUILD_FUNCTORCH, USE_NUMPY) have to be passed
  # here, along with the install prefix setup.py expects. Note that this leaves those options on in
  # the cache, so a subsequent --fast C++ rebuild will also build the Python targets.
  if [[ -f "$source_dir/build/CMakeCache.txt" ]]; then
    local cmake_generator
    cmake_generator=$(sed -n 's/^CMAKE_GENERATOR:INTERNAL=//p' "$source_dir/build/CMakeCache.txt")
    if [[ -z "$cmake_generator" ]]; then
      >&2 echo "ERROR: No CMAKE_GENERATOR in $source_dir/build/CMakeCache.txt"
      return 1
    fi

    # setup.py picks the Ninja generator whenever ninja is on PATH, which installing the
    # requirements above can make true even when this build directory was configured with a
    # different generator. It takes that decision from CMAKE_GENERATOR, so pinning the generator
    # that already owns the cache is what keeps it from reconfiguring with a mismatched one.
    export CMAKE_GENERATOR="$cmake_generator"
    unset CMAKE_FRESH
    cmake \
      -S "$source_dir" \
      -B "$source_dir/build" \
      -DBUILD_PYTHON=ON \
      -DBUILD_FUNCTORCH=ON \
      -DUSE_NUMPY=ON \
      -DPython_EXECUTABLE="$python_executable" \
      -DCMAKE_INSTALL_PREFIX="$source_dir/torch" || return
  else
    export CMAKE_FRESH=1
  fi

  cd "$source_dir" || return
  "${PIP_EXE[@]}" install --no-cache --no-build-isolation .
)
