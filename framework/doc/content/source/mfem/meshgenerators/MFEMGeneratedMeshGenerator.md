# MFEMGeneratedMeshGenerator

!if! function=hasCapability('mfem')

## Overview

`MFEMGeneratedMeshGenerator` generates a structured Cartesian MFEM mesh for use in an
`MFEMProblem`. It produces a line (1D), rectangle (2D), or box (3D) with uniformly spaced
elements, and is the MFEM analog of [GeneratedMeshGenerator.md].

The [!param](/Mesh/MFEMGeneratedMeshGenerator/dim) parameter is required and selects the spatial
dimension. Element type defaults to `QUAD` for 2D and `HEX` for 3D; `TRI` and `TET` are also
supported via [!param](/Mesh/MFEMGeneratedMeshGenerator/elem_type).

Named boundary sets are assigned automatically so boundaries can be referenced by name in
`[BCs]` blocks:

| Dimension | Boundary names |
|-----------|----------------|
| 1D | `left`, `right` |
| 2D | `bottom`, `right`, `top`, `left` |
| 3D | `bottom`, `front`, `right`, `back`, `left`, `top` |

## Example Input File Syntax

!listing test/tests/mfem/meshgenerators/generated/test.i block=Mesh BCs

!syntax parameters /Mesh/MFEMGeneratedMeshGenerator

!syntax inputs /Mesh/MFEMGeneratedMeshGenerator

!syntax children /Mesh/MFEMGeneratedMeshGenerator

!if-end!

!else
!include mfem/mfem_warning.md
