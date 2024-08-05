!template load file=sll.md.template category=framework project=MOOSE

## libMesh

[!ac](MOOSE) relies on libMesh, which is summarized as follows on the project website:
[libmesh.github.io](https://libmesh.github.io).

> The libMesh library provides a framework for the numerical simulation of partial differential
> equations using arbitrary unstructured discretizations on serial and parallel platforms. A major goal
> of the library is to provide support for adaptive mesh refinement (AMR) computations in parallel
> while allowing a research scientist to focus on the physics they are modeling.

+Testing:+ libMesh is a core library providing significant functionality for MOOSE and MOOSE-based
applications. Every Functional test in the [framework RTM](framework_rtm.md) that manipulates a mesh
or solves a problem is using an integrated piece of the libMesh library. For all practical purposes
there are almost no differences between the set of tests for the framework and the set of tests for libMesh.

Current Version of libMesh: [!git!submodule-hash url=https://github.com/libmesh/libmesh/commit](libmesh)

## [!ac](PETSc)

[!ac](MOOSE) relies on [!ac](PETSc), which is summarized as follows on the project website:
[www.mcs.anl.gov/petsc](https://www.mcs.anl.gov/petsc/).

> [!ac](PETSc), pronounced PET-see (the S is silent), is a suite of data structures and routines for the
> scalable (parallel) solution of scientific applications modeled by partial differential equations. It
> supports MPI, and GPUs through CUDA or OpenCL, as well as hybrid MPI-GPU parallelism. PETSc
> (sometimes called PETSc/Tao) also contains the Tao optimization software library.
>

+Testing:+ PETSc is a core library providing significant functionality for MOOSE and MOOSE-based
applications. Every Functional test in the [framework RTM](framework_rtm.md) that
solves a problem is using an integrated piece of the PETSc library. For all practical purposes
there are almost no differences between the set of tests for the framework and the set of tests for PETSc.


Current Version of [!ac](PETSc): [!git!submodule-hash url=https://github.com/petsc/petsc/commit](petsc)

## HYPRE (optional)

[!ac](MOOSE) can optionally use preconditioners from the HYPRE library to assist with the solvers
in [!ac](PETSc). The BoomerAMG preconditioner is frequeuntly used in MOOSE for elliptic PDEs.
This project is summarized as follows on the project website:
[https://computing.llnl.gov/projects/hypre-scalable-linear-solvers-multigrid-methods](https://computing.llnl.gov/projects/hypre-scalable-linear-solvers-multigrid-methods).

> Livermore’s hypre library of linear solvers makes possible larger, more detailed simulations by solving
> problems faster than traditional methods at large scales. It offers a comprehensive suite of scalable
> solvers for large-scale scientific simulation, featuring parallel multigrid methods for both structured
> and unstructured grid problems. The hypre library is highly portable and supports a number of languages.

+Testing:+ [!ac](PETSc) is able to utilize several externally-developed preconditioners in its solution
scheme. Explicit testing of the Hypre BoomerAMG preconditioner is not done by the MOOSE suite since it
is an optional dependency and isn't strictly required to solve MOOSE's linear and nonlinear systems.
However, this preconditioner option does appear in a number of the Functional tests in the
[framework RTM](framework_rtm.md), such that breaking changes would likely result in differences in many
of MOOSE's tests.

## SuperLU (optional)

[!ac](MOOSE) can optionally use the SuperLU "solver" package for accelerating the factorization of
certain types of linear systems. This package is summarized as follows on the project website:

> SuperLU is a general-purpose library for the direct solution of large, sparse, nonsymmetric systems of
> linear equations. The library is written in C and is callable from either C or Fortran program. It uses
> MPI, OpenMP and CUDA to support various forms of parallelism. It supports both real and complex datatypes,
> both single and double precision, and 64-bit integer indexing. The library routines perform an LU
> decomposition with partial pivoting and triangular system solves through forward and back substitution.
> The LU factorization routines can handle non-square matrices but the triangular solves are performed only
> for square matrices.

+Testing:+ [!ac](PETSc) is able to utilize the SuperLU package to solve linear system matrices in parallel.
This acceleration is suitable for certain small-to-moderately-sized linear systems that are not easily
solved with other schemes. Explicit testing of the SuperLU factorization package is not done by the MOOSE
suite since it is an optional dependency. However, this solver option does appear in a number of
Functional tests in the [framework RTM](framework_rtm.md) and module RTMs. Breaking changes
would likely result in differences in many of these tests.

## MUMPS (optional)

[!ac](MOOSE) can optionally use the MUMPS "solver" package for accelerating the factorization of
certain types of linear systems. This package is summarized as follows on the project website:

> MUMPS Main Features:
>
> - Solution of large linear systems with symmetric positive definite matrices general symmetric matrices general unsymmetric matrices.
> - Real or complex arithmetic (single or double precision)
> - Parallel factorization and solve phases (uniprocessor version also available)
> - Out of core numerical phases
> - Iterative refinement and backward error analysis
> - Various matrix input formats assembled, distributed, elemental format
> - Partial factorization and Schur complement matrix (centralized or 2D block-cyclic) with reduced/condensed right-hand side
> - Interfaces to MUMPS: Fortran, C, Matlab and Scilab
> - Several reorderings interfaced: AMD, QAMD, AMF, PORD, METIS, PARMETIS, SCOTCH, PT-SCOTCH
> - Symmetric indefinite matrices: preprocesssing and 2-by-2 pivots
> - Parallel analysis and matrix scaling
> - Computation of the determinant (with an option to discard factors)
> - Forward elimination during factorization 
>

+Testing:+ [!ac](PETSc) is able to utilize the MUMPS package to solve linear system matrices in parallel.
This acceleration is suitable for certain small-to-moderately-sized linear systems that are not easily
solved with other schemes. Explicit testing of the MUMPS factorization package is not done by the MOOSE
suite since it is an optional dependency. However, this solver option does appear in a number of
Functional tests in the [framework RTM](framework_rtm.md) and module RTMs. Breaking changes
would likely result in differences in many of these tests.
