# MFEMMesh

!if! function=hasCapability('mfem')

## Summary

Abstract base class shared by the MFEM-backed mesh types.

## Overview

`MFEMMesh` holds the `mfem::ParMesh` used by an [MFEMProblem.md] and implements the operations
common to all MFEM-backed meshes: refinement, reordering, partitioning, displacement, and
recovery. It is not used directly; the concrete mesh types are [MFEMFileMesh.md], which reads a
mesh from file, and [MFEMMeshGeneratorMesh.md], which receives the mesh produced by a chain of
MFEM mesh generators.

As MOOSE checks for the existence of a libMesh MOOSE mesh at various points during setup,
`MFEMMesh` also builds a small libMesh placeholder mesh alongside the MFEM mesh. This placeholder
is not the simulation mesh; all MFEM objects should access the `mfem::ParMesh` via the
`getMFEMParMesh()` accessor as needed.

!if-end!

!else
!include mfem/mfem_warning.md
