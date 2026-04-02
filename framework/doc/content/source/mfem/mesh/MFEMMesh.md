# MFEMMesh

!if! function=hasCapability('mfem')

## Overview

`MFEMMesh` builds an `mfem::ParMesh` object from the provided mesh input file
for use in an `MFEMProblem`. Exodus files are supported, along with other mesh formats listed
 [here](https://mfem.org/mesh-formats/).

As MOOSE checks for the existence of a `libMesh` MOOSE mesh at various points during setup,
`MFEMMesh` currently builds a dummy MOOSE mesh of a single point alongside the MFEM mesh. This dummy
mesh should not be used in an `MFEMProblem`; all MFEM objects should access the `mfem::ParMesh` via
the `getMFEMParMesh()` accessor as needed.

It is not necessary to use an `MFEMMesh` with an
`MFEMProblem`. `MFEMProblem` is also capable of creating an
`mfem::ParMesh` from a `libMesh`-based mesh, although not all element
types are currently supported. For more information on this conversion
process and its limitations, see the [documentation on
`buildMFEMMesh`](source/mfem/utils/BuildMFEMMesh.md).

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=Problem Mesh

!syntax parameters /Mesh/MFEMMesh

!syntax inputs /Mesh/MFEMMesh

!syntax children /Mesh/MFEMMesh

!if-end!

!else
!include mfem/mfem_warning.md
