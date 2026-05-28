## Required Dependencies id=required_dependencies

The required dependencies for MOOSE are as follows:

### C and C++17 compiler

A compiler for building C and C++ code. The preferred compilers are:

- [GCC](https://gcc.gnu.org/): tested on versions [!package!gcc_all]
- [Clang](https://clang.llvm.org/): tested on versions [!package!clang_all]

### Fortran compiler

A compiler for building Fortran code. The preferred compiler is:

- [GCC](https://gcc.gnu.org/): tested on versions [!package!gcc_all]

### GNU Make

[GNU Make](https://www.gnu.org/software/make) is used to execute `Makefile` files for building applications.

### MPI

Message Passing Interface (MPI) is a portable message-passing standard to execute applications in parallel. The preferred implementations are:

- [MPICH](https://www.mpich.org/): tested on versions [!package!mpich_all]
- [OpenMPI](https://www.open-mpi.org/): tested on version [!package!openmpi_all]

### PETSc

[PETSc](https://petsc.org/), the Portable, Extensible Toolkit for Scientific Computation, is for the scalable (parallel) solution of scientific applications modeled by partial differential equations (PDEs).

The currently supported version is [!versioner!version prefix=v url=https://gitlab.com/petsc/petsc/-/tags/ package=petsc]. Other versions are not guaranteed to be compatible.

Pre-built distributions of PETSc are found in the distributed development environments via Apptainer, Conda, and Docker. Thus, PETSc does not need to be built with these environments.

If not using pre-built development environments from Apptainer, Conda, and Docker, PETSc can be installed via the [scripts/update_and_rebuild_petsc.sh language=bash] script within the MOOSE repository. By default, this script will install PETSc to `./petsc/arch-moose` relative to the MOOSE repository root.

### libMesh

[libMesh](https://libmesh.github.io/) provides a framework for mesh definition and finite-element assembly.

The currently supported commit is [!git!submodule-hash length=7 url=https://github.com/libMesh/libmesh/commit](libmesh). Other commits are not guaranteed to be compatible.

Pre-built distributions of libMesh are found in the distributed development environments via Apptainer, Conda, and Docker. Thus, libMesh does not need to be built with these environments.

If not using pre-built development environments from Apptainer, Conda, and Docker, PETSc can be installed via the [scripts/update_and_rebuild_libmesh.sh language=bash] script within the MOOSE repository. By default, this script will install libMesh to `./libmesh/installed` relative to the MOOSE repository root. This script requires PETSc to also be installed, where the PETSc directory is defined by the environment variable `$PETSC_DIR` or is installed in `./petsc/arch-moose` relative to the MOOSE repository root.

### Python

Python versions [!package!python_all] are supported.

### WASP

[WASP](https://code.ornl.gov/neams-workbench/wasp) is used as the backend for parsing MOOSE input files and also provides the language server for interacting with MOOSE input.

The currently supported commit is [!git!submodule-hash length=7 url=https://code.ornl.gov/neams-workbench/wasp/-/commit](framework/contrib/wasp). Other commits are not guaranteed to be compatible.

Pre-built distributions of PETSc are found in the distributed development environments via Apptainer, Conda, and Docker. Thus, WASP does not need to be built with these environments.

If not using pre-built development environments from Apptainer, Conda, and Docker, WASP can be installed via the [scripts/update_and_rebuild_wasp.sh language=bash] script within the MOOSE repository.  This script requires CMake for configuration. By default, this script will install WASP to `./framework/contrib/wasp/install` relative to the MOOSE repository root.
