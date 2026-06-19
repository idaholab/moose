# Computational Backends and Accelerated Components

MOOSE has several computational paths that share the same input-file-driven framework, but they do
not all replace the same layer of the stack. Some are alternative finite element backends, some are
portable implementations of MOOSE object systems, and some connect MOOSE to specialized external
libraries.

This page gives a high-level map of those paths and points to the detailed documentation for each
one.

## Framework view

The default MOOSE path uses MOOSE objects, libMesh data structures, and PETSc solvers. Optional
capabilities can replace or accelerate parts of that path:

```text
MOOSE application and input file
|
+-- Default MOOSE/libMesh path
|   +-- FE/FV variables, Kernels, BCs, Materials, UserObjects
|   +-- libMesh mesh and finite element data structures
|   +-- PETSc nonlinear and linear solvers
|
+-- Kokkos-MOOSE
|   +-- Kokkos implementations of selected MOOSE object systems
|   +-- GPU-portable assembly and execution for supported objects
|
+-- MFEM-MOOSE
|   +-- MFEMProblem, MFEMMesh, MFEM variables, kernels, BCs, and solvers
|   +-- MFEM finite element spaces and assembly levels
|   +-- Optional GPU and libCEED execution paths through MFEM
|
+-- NEML2
    +-- External material models connected through the [NEML2] block
    +-- Optional device-aware material evaluation
```

## Choosing a path

| Path | What it changes | Use it when | Main entry points |
| :- | :- | :- | :- |
| Default MOOSE/libMesh | The standard MOOSE finite element and finite volume path | You need broad MOOSE capability, mature object coverage, or CPU-focused development | [Problem](syntax/Problem/index.md), [Kernels](syntax/Kernels/index.md), [Materials](syntax/Materials/index.md), [Executioner](syntax/Executioner/index.md) |
| Kokkos-MOOSE | Implementations of selected MOOSE object systems for portable device execution | You want GPU-portable versions of supported MOOSE objects while keeping the MOOSE object model | [Kokkos-MOOSE](syntax/Kokkos/index.md), [Kokkos Kernels](syntax/KokkosKernels/index.md), [Kokkos Materials](syntax/KokkosMaterials/index.md) |
| MFEM-MOOSE | The finite element backend and the associated problem, mesh, variable, kernel, solver, and output objects | You need MFEM features such as high-order spaces, H(div)/H(curl) spaces, partial assembly, or MFEM GPU support | [MFEM-MOOSE](syntax/MFEM/index.md), [MFEMProblem](source/mfem/problem/MFEMProblem.md), [FESpaces](syntax/FESpaces/index.md) |
| NEML2 | Material model evaluation, not the finite element backend | You want MOOSE to call NEML2 material models and transfer inputs, outputs, and derivatives | [NEML2 syntax](syntax/NEML2/index.md) |
| libCEED | Operator evaluation underneath MFEM, not a separate MOOSE object model | You are using MFEM-MOOSE and want libCEED-backed matrix-free execution where supported | [MFEM-MOOSE](syntax/MFEM/index.md) |

## Default MOOSE/libMesh path

Most MOOSE applications use the default MOOSE/libMesh path. In this mode, application developers
derive from the standard MOOSE object families, such as `Kernel`, `ADKernel`, `IntegratedBC`,
`Material`, `UserObject`, `AuxKernel`, and related systems. MOOSE manages object construction,
dependency ordering, residual and Jacobian assembly, output, restart, transfers, and execution.

This path is the broadest and most mature route through the framework. It is usually the right
starting point unless a problem specifically requires a newer accelerated path or a capability that is
only available through another backend.

## Kokkos-MOOSE

Kokkos-MOOSE ports selected MOOSE object systems to GPU-portable execution while retaining the
general MOOSE programming model. A developer typically chooses Kokkos-MOOSE by using Kokkos-specific
object families, such as `Moose::Kokkos::Kernel`, `Moose::Kokkos::IntegratedBC`,
`Moose::Kokkos::Material`, or the corresponding Kokkos syntax pages.

Kokkos-MOOSE is not a separate finite element backend in the same sense as MFEM-MOOSE. It is a
device-portable implementation path for supported MOOSE systems, built around Kokkos data structures,
memory-space rules, and parallel dispatch.

For details, see [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md).

## MFEM-MOOSE

MFEM-MOOSE uses [MFEM](https://mfem.org) as its finite element backend. Inputs use MFEM-specific
problem, mesh, finite element space, variable, kernel, boundary condition, solver, executioner, and
output objects. This path is useful for capabilities that are natural in MFEM, including high-order
finite elements, H(div) and H(curl) spaces, partial assembly, and low-order-refined solvers.

MFEM-MOOSE is the path that most directly changes the finite element backend. It is also the MOOSE
path where libCEED-backed execution appears: when MFEM-MOOSE is built with libCEED support, MFEM
assembly and device options can use libCEED for supported matrix-free operations.

For details, see [Getting started with MFEM-MOOSE](syntax/MFEM/index.md).

## NEML2

NEML2 connects MOOSE simulations to NEML2 material models. It does not replace the MOOSE problem,
mesh, finite element, or solver stack. Instead, the `[NEML2]` block defines how MOOSE transfers
quantities into NEML2, retrieves outputs, and stores those outputs as MOOSE material properties.

Use NEML2 when the constitutive model itself should live in NEML2 or use NEML2 device scheduling. A
simulation can still otherwise follow the default MOOSE path or another compatible host path.

For details, see [NEML2 syntax](syntax/NEML2/index.md).

## libCEED

libCEED is best understood as an acceleration layer used underneath supported finite element
operations, not as a user-facing MOOSE object family. In MOOSE, libCEED is currently exposed through
MFEM-MOOSE configuration. For example, MFEM executioners support assembly levels, and the `none`
assembly level is available when MFEM-MOOSE has been built with libCEED support.

Users generally should not start by asking whether to derive from libCEED classes. They should first
decide whether their problem belongs on the MFEM-MOOSE path, then select the MFEM assembly and device
settings appropriate for their build and hardware.

## Practical guidance

Start with the default MOOSE/libMesh path when you need broad framework coverage or are building a
new MOOSE application. Move to Kokkos-MOOSE when the relevant object systems are supported and GPU
portability is the main goal. Move to MFEM-MOOSE when the finite element formulation, spaces, assembly
strategy, or solver path is specifically an MFEM fit. Use NEML2 when the material model should be
owned by NEML2. Treat libCEED as an execution option for supported MFEM-MOOSE runs rather than as a
separate application development model.
