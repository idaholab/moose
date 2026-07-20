# Computational Backends

A modeler can choose among several MOOSE computational backends depending on the problem being
solved and the hardware being targeted. Some use MOOSE object systems with libMesh data structures,
some provide device-portable implementations of selected MOOSE object systems, and some use a
different finite element backend.

This page gives a high-level map of those choices and points to the detailed documentation for each
backend.

## Framework view

The traditional MOOSE/libMesh path uses MOOSE objects, libMesh data structures, and PETSc solvers.
Other capabilities provide alternatives for parts of that path:

```text
MOOSE application and input file
|
+-- MOOSE/libMesh path
|   +-- FE/FV variables, Kernels, BCs, Materials, UserObjects
|   +-- libMesh mesh and finite element data structures
|   +-- PETSc nonlinear and linear solvers
|
+-- Kokkos-MOOSE
|   +-- Kokkos implementations of selected MOOSE object systems
|   +-- libMesh finite element backend, MeshBase-derived mesh, and DofMap
|   +-- GPU-portable assembly and execution for supported objects
|   +-- Compatible with MOOSE/libMesh systems with a few exceptions
|
+-- MFEM-MOOSE
|   +-- MFEMProblem, MFEMMesh, MFEM variables, kernels, BCs, and solvers
|   +-- MFEM finite element spaces and assembly levels
|   +-- Optional GPU and libCEED execution paths through MFEM
```

## Choosing a path

| Path | What it changes | Use it when | Starting points |
| :- | :- | :- | :- |
| MOOSE/libMesh | MOOSE finite element and finite volume object systems using libMesh data structures | You want the broadest object coverage, module compatibility, examples, and application-development support | [Application development tutorial](getting_started/examples_and_tutorials/tutorial01_app_development/index.md optional=True), [framework examples](getting_started/examples_and_tutorials/examples_tutorials.md#framework-examples optional=True) |
| Kokkos-MOOSE | Device-portable implementations of selected MOOSE object systems using the libMesh finite element backend | You want supported kernels, boundary conditions, and materials to run through Kokkos while retaining MOOSE meshes, postprocessing, and output workflows | [Getting started with Kokkos-MOOSE](syntax/Kokkos/index.md), [Kokkos installation](getting_started/installation/install_kokkos.md optional=True) |
| MFEM-MOOSE | MFEM-backed problem, mesh, variable, finite element space, kernel, solver, and output objects | You want to formulate the application using MFEM features such as high-order spaces, H(div)/H(curl) spaces, or partial assembly | [Getting started with MFEM-MOOSE](syntax/MFEM/index.md), [MFEM installation](getting_started/installation/install_mfem.md optional=True) |

## MOOSE/libMesh path

Most MOOSE applications use MOOSE/libMesh objects. In this mode, application developers create
objects derived from MOOSE object families, such as `Kernel`, `ADKernel`, `IntegratedBC`,
`Material`, `UserObject`, `AuxKernel`, and related systems.

For guidance on using the MOOSE/libMesh backend, see the
[application development tutorial](getting_started/examples_and_tutorials/tutorial01_app_development/index.md  optional=True)
and the [framework examples](getting_started/examples_and_tutorials/examples_tutorials.md#framework-examples  optional=True).

## Kokkos-MOOSE

Kokkos-MOOSE ports selected MOOSE object systems to GPU-portable execution. It continues to use
libMesh `MeshBase` and `DofMap` data structures. A developer typically chooses Kokkos-MOOSE by using
Kokkos-specific object families, such as `Moose::Kokkos::Kernel`, `Moose::Kokkos::IntegratedBC`,
`Moose::Kokkos::Material`, or the corresponding Kokkos syntax pages.

Kokkos-MOOSE is not a separate finite element backend in the same sense as MFEM-MOOSE. It is a
device-portable implementation path for supported MOOSE systems, built around Kokkos data structures,
memory-space rules, and parallel dispatch. It relies on libMesh as the finite element backend. Many
Kokkos-MOOSE system objects are compatible with host MOOSE/libMesh objects, but exceptions exist; for
example, host MOOSE nodal boundary conditions cannot be used in the same input as Kokkos-MOOSE
residual objects.

For details, see [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md).

## MFEM-MOOSE

MFEM-MOOSE uses [MFEM](https://mfem.org) as its finite element backend. Inputs use MFEM-specific
problem, mesh, finite element space, variable, kernel, boundary condition, solver, executioner, and
output objects. This path is useful for capabilities that are natural in MFEM, including high-order
finite elements, H(div) and H(curl) spaces, partial assembly, and low-order-refined solvers. When
MFEM-MOOSE is built with libCEED support, MFEM can provide matrix-free operator application through
libCEED.

For details, see [Getting started with MFEM-MOOSE](syntax/MFEM/index.md).
