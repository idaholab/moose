# MFEMScalarIC

!if! function=hasCapability('mfem')

## Summary

!syntax description /ICs/MFEMScalarIC

## Overview

`MFEMScalarIC` is used to set initial conditions of scalar valued MFEM variables from an
`mfem::Coefficient`. The initial condition will be applied on all subdomains in the mesh.

## Example Input File Syntax

!listing mfem/ics/scalar_ic.i block=ICs

!syntax parameters /ICs/MFEMScalarIC

!syntax inputs /ICs/MFEMScalarIC

!syntax children /ICs/MFEMScalarIC

!if-end!

!else
!include mfem/mfem_warning.md
