# MFEMComplexVectorIC

!if! function=hasCapability('mfem')

## Overview

`MFEMComplexVectorIC` is used to set initial conditions of complex vector valued MFEM variables from two
`mfem::VectorCoefficient`s representing its real and imaginary parts. The initial condition will be applied on all subdomains in the mesh.

## Example Input File Syntax

!listing mfem/complex/mixed_sesquilinear_ic.i block=ICs

!syntax parameters /ICs/MFEMComplexVectorIC

!syntax inputs /ICs/MFEMComplexVectorIC

!syntax children /ICs/MFEMComplexVectorIC

!if-end!

!else
!include mfem/mfem_warning.md
