# MFEMComplexVectorIC

!if! function=hasCapability('mfem')

## Overview

`MFEMComplexVectorIC` is used to set initial conditions of complex vector valued MFEM variables from two
`mfem::VectorCoefficient`s representing its real and imaginary parts. The initial condition will be applied on all subdomains in the mesh.

## Syntax

!syntax parameters /ICs/MFEMComplexVectorIC

!syntax inputs /ICs/MFEMComplexVectorIC

!syntax children /ICs/MFEMComplexVectorIC

!if-end!

!else
!include mfem/mfem_warning.md
