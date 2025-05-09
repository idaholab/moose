# MFEMSubMeshTransfer

!if! function=hasCapability('mfem')

## Summary

!syntax description /Transfers/MFEMSubMeshTransfer

## Overview

`MFEMSubMeshTransfer` transfers data between [MFEM variables](MFEMVariable.md) sharing a common
subspace within the same problem, when at least one is defined on an 
[MFEM SubMesh](MFEMSubMeshBase.md). 
The finite element space of the MFEM variables must otherwise be of the same type.

!syntax parameters /Transfers/MFEMSubMeshTransfer

!syntax inputs /Transfers/MFEMSubMeshTransfer

!syntax children /Transfers/MFEMSubMeshTransfer

!if-end!

!else
!include mfem/mfem_warning.md
