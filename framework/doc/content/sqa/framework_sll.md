!template load file=sll.md.template category=framework project=MOOSE

## libMesh

[!ac](MOOSE) relies on libMesh, which is summarized as follows on the project website:
[libmesh.github.io](https://libmesh.github.io).

> The libMesh library provides a framework for the numerical simulation of partial differential
> equations using arbitrary unstructured discretizations on serial and parallel platforms. A major goal
> of the library is to provide support for adaptive mesh refinement (AMR) computations in parallel
> while allowing a research scientist to focus on the physics they are modeling.

Current Version of libMesh: [!git!submodule-hash url=https://github.com/libmesh/libmesh/commit](libmesh)

## [!ac](PETSc)

[!ac](MOOSE) relies on [!ac](PETSc), which is summarized as follows on the project website:
[www.mcs.anl.gov/petsc](https://www.mcs.anl.gov/petsc/).

> [!ac](PETSc), pronounced PET-see (the S is silent), is a suite of data structures and routines for the
> scalable (parallel) solution of scientific applications modeled by partial differential equations. It
> supports MPI, and GPUs through CUDA or OpenCL, as well as hybrid MPI-GPU parallelism. PETSc
> (sometimes called PETSc/Tao) also contains the Tao optimization software library.

Current Version of [!ac](PETSc): [!git!submodule-hash url=https://github.com/petsc/petsc/commit](petsc)
