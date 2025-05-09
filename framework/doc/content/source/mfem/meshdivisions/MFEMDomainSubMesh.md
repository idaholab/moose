# MFEMDomainSubMesh

!if! function=hasCapability('mfem')

## Summary

!syntax description /SubMeshes/MFEMDomainSubMesh

## Overview

`MFEMDomainSubMesh` specifies and builds an `mfem::ParSubMesh` object from a volumetric subspace of
the parent `mfem::ParMesh` used in the `MFEMProblem`, from a user-specified set of blocks.

## Example Input File Syntax

!listing test/tests/mfem/submeshes/domain_submesh.i block=SubMeshes

!syntax parameters /SubMeshes/MFEMDomainSubMesh

!syntax inputs /SubMeshes/MFEMDomainSubMesh

!syntax children /SubMeshes/MFEMDomainSubMesh

!if-end!

!else
!include mfem/mfem_warning.md
