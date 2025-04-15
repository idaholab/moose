# SetMFEMMeshFESpaceAction

!if! function=hasCapability('mfem')

## Summary

Sets the nodal mesh FESpace to use the same FESpace as the
[`MFEMVariable`](source/mfem/variables/MFEMVariable.md) used to describe node displacements.

## Overview

Action called to set the nodal mesh FESpace to use the same FESpace as the variable used to describe
node displacements in problems involving deformed meshes, parsing content inside the `Mesh` block in
the user input. Only has an effect if the `Problem` type is set to
[`MFEMProblem`](source/mfem/problem/MFEMProblem.md), the `Mesh` type is set to [`MFEMMesh`](source/mfem/mesh/MFEMMesh.md)
and the `displacement` field is set.

## Example Input File Syntax

!listing test/tests/mfem/kernels/linearelasticity.i block=Mesh

!if-end!

!else
!include mfem/mfem_warning.md
