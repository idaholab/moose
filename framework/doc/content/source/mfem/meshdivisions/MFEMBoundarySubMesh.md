# MFEMBoundarySubMesh

!if! function=hasCapability('mfem')

## Summary

!syntax description /SubMeshes/MFEMBoundarySubMesh

## Overview

`MFEMBoundarySubMesh` specifies and builds an `mfem::ParSubMesh` object from a surface subspace of
the parent `mfem::ParMesh` used in the `MFEMProblem`, from a user-specified set of boundaries.

## Example Input File Syntax

!listing test/tests/mfem/submeshes/boundary_submesh.i block=SubMeshes

!syntax parameters /SubMeshes/MFEMBoundarySubMesh

!syntax inputs /SubMeshes/MFEMBoundarySubMesh

!syntax children /SubMeshes/MFEMBoundarySubMesh

!if-end!

!else !include mfem/mfem_warning.md
