# MFEMSubMeshBase

!if! function=hasCapability('mfem')

## Summary

Base class for objects that specify and build an MFEM SubMesh to add to an `MFEMProblem`.

## Overview

MFEM SubMeshes are responsible for building an `mfem::ParSubMesh` object, from a user-specified
subspace of the parent `mfem::ParMesh` used in the `MFEMProblem`. Each `mfem::ParSubMesh` is itself
an `mfem::ParMesh`, and as such, can be used by downstream objects that are defined on an MFEM mesh,
such as `MFEMFESpaces`.

MFEM SubMeshes are particularly useful for restricting the domains of `MFEMFESpaces` in mixed
problems where some `MFEMVariables` do not need to be defined across the entire mesh, to reduce
problem size and improve conditioning. [MFEM Examples 34 and 35](https://mfem.org/examples/) give
some examples of problems in which they can be used.

`MFEMSubMeshBase` is a virtual base class. Derived classes should override the `buildSubMesh` method
to construct the desired `mfem::ParSubMesh` from the parent mesh.

!if-end!

!else !include mfem/mfem_warning.md
