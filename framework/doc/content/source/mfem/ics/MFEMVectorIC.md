# MFEMVectorIC

!if! function=hasCapability('mfem')

## Summary

!syntax description /ICs/MFEMVectorIC

## Overview

`MFEMVectorIC` is used to set initial conditions of vector valued MFEM variables from an
`mfem::VectorCoefficient`. The initial condition will be applied on all subdomains in the mesh.

## Example Input File Syntax

!listing mfem/ics/vector_ic.i block=ICs

!syntax parameters /ICs/MFEMVectorIC

!syntax inputs /ICs/MFEMVectorIC

!syntax children /ICs/MFEMVectorIC

!if-end!

!else
!include mfem/mfem_warning.md
