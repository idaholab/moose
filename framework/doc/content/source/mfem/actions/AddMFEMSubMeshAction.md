# AddMFEMSubMeshAction

!if! function=hasCapability('mfem')

## Summary

!syntax description /SubMeshes/AddMFEMSubMeshAction

## Overview

Action called to add an MFEM finite element space to the problem, parsing content inside a
[`SubMeshes`](source/mfem/meshdivisions/MFEMSubMeshBase.md) block in the user input. Only has an
effect if the `Problem` type is set to [`MFEMProblem`](source/mfem/problem/MFEMProblem.md).

## Example Input File Syntax

!listing test/tests/mfem/submeshes/domain_submesh.i block=Problem SubMeshes

!syntax parameters /SubMeshes/AddMFEMSubMeshAction

!if-end!

!else
!include mfem/mfem_warning.md
