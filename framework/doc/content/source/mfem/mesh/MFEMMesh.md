# MFEMMesh

!if! function=hasCapability('mfem')

## Summary

!syntax description /Mesh/MFEMMesh

## Overview

`MFEMMesh` is responsible for building an `mfem::ParMesh` object from the provided mesh input file
for use in an `MFEMProblem`. Exodus files are supported, along with other mesh formats listed
 [here](https://mfem.org/mesh-formats/).

As MOOSE checks for the existence of a `libMesh` MOOSE mesh at various points during setup,
`MFEMMesh` currently builds an dummy MOOSE mesh of a single quad alongside the MFEM mesh. This dummy
mesh should not be used in an `MFEMProblem`; all MFEM objects should access the `mfem::ParMesh` via
the `getMFEMParMesh()` accessor as needed.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=Problem Mesh

!syntax parameters /Mesh/MFEMMesh

!syntax inputs /Mesh/MFEMMesh

!syntax children /Mesh/MFEMMesh

!if-end!

!else
!include mfem/mfem_warning.md
