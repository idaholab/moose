# MFEMScalarBoundaryIC

!if! function=hasCapability('mfem')

## Summary

!syntax description /ICs/MFEMScalarBoundaryIC

## Overview

`MFEMScalarBoundaryIC` is used to set initial conditions of scalar valued MFEM variables from an
`mfem::Coefficient`. The initial condition will be applied on all boundaries specified by the user
in the mesh.

## Example Input File Syntax

!listing mfem/submeshes/cut_closed_coil.i block=ICs

!syntax parameters /ICs/MFEMScalarBoundaryIC

!syntax inputs /ICs/MFEMScalarBoundaryIC

!syntax children /ICs/MFEMScalarBoundaryIC

!if-end!

!else
!include mfem/mfem_warning.md
