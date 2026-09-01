# MFEMGeneratedMeshGenerator

!if! function=hasCapability('mfem')

## Overview

`MFEMGeneratedMeshGenerator` generates a structured Cartesian MFEM mesh for use in an
`MFEMProblem`. It produces a line (1D), rectangle (2D), or box (3D) with uniformly spaced
elements, and is the MFEM analog of [GeneratedMeshGenerator.md].

The [!param](/Mesh/MFEMGeneratedMeshGenerator/dim) parameter is required and selects the spatial
dimension. `EDGE` is the only element type for 1D meshes. Element type defaults to
`QUAD` for 2D and `HEX` for 3D; `TRI` and `TET` are also supported
via [!param](/Mesh/MFEMGeneratedMeshGenerator/elem_type).

Named boundary sets are assigned automatically so boundaries can be referenced by name in
`[BCs]` blocks:

| Dimension | Boundary names (ID) |
|-----------|----------------|
| 1D | `left` (1), `right` (2) |
| 2D | `bottom` (1), `right` (2), `top` (3), `left` (4) |
| 3D | `bottom` (1), `front` (2), `right` (3), `back` (4), `left` (5), `top` (6) |

## Example Input File Syntax

!listing test/tests/mfem/meshgenerators/generated/test.i block=Mesh BCs

!syntax parameters /Mesh/MFEMGeneratedMeshGenerator

!syntax inputs /Mesh/MFEMGeneratedMeshGenerator

!syntax children /Mesh/MFEMGeneratedMeshGenerator

!if-end!

!else
!include mfem/mfem_warning.md
