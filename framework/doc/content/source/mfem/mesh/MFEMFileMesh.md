# MFEMFileMesh

!if! function=hasCapability('mfem')

## Overview

`MFEMFileMesh` reads an `mfem::ParMesh` from file for use in an `MFEMProblem`. Exodus files are
supported, along with other mesh formats listed [here](https://mfem.org/mesh-formats/).

As MOOSE checks for the existence of a `libMesh` MOOSE mesh at various points during setup,
`MFEMFileMesh` builds a dummy MOOSE mesh of a single point alongside the MFEM mesh. This dummy
mesh should not be used in an `MFEMProblem`; all MFEM objects should access the `mfem::ParMesh` via
the `getMFEMParMesh()` accessor as needed.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=Problem Mesh

!syntax parameters /Mesh/MFEMFileMesh

!syntax inputs /Mesh/MFEMFileMesh

!syntax children /Mesh/MFEMFileMesh

!if-end!

!else
!include mfem/mfem_warning.md
