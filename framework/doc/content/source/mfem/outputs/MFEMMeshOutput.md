# MFEMMeshOutput

!if! function=hasCapability('mfem')

!syntax description /Outputs/MFEMMeshOutput

## Overview

`MFEMMeshOutput` gathers the `mfem::ParMesh` of an [MFEMProblem.md], or of one of its
submeshes, onto a single process and writes it to a `.mesh` file in the native MFEM mesh
format.

Before writing, the mesh is put into a canonical form: vertices are numbered in order of their
coordinates, the vertices of each tetrahedron are listed in a fixed rotation, and elements are
sorted by their vertices. A mesh is therefore written identically however it was numbered or
partitioned, which allows meshes obtained in different ways to be compared directly. The
elements can then optionally be reordered along a Hilbert curve or with the Gecko library using
the `ordering` parameter.

## Example Input File Syntax

!listing test/tests/mfem/mesh/libmesh_mfem_conversion/libmesh_parent.i block=Outputs

!syntax parameters /Outputs/MFEMMeshOutput

!syntax inputs /Outputs/MFEMMeshOutput

!syntax children /Outputs/MFEMMeshOutput

!if-end!

!else
!include mfem/mfem_warning.md
