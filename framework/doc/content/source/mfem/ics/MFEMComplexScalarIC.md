# MFEMComplexScalarIC

!if! function=hasCapability('mfem')

## Overview

`MFEMComplexScalarIC` is used to set initial conditions of complex scalar valued MFEM variables from two
`mfem::Coefficient`s representing its real and imaginary parts. The initial condition will be applied on all subdomains in the mesh.

## Example Input File Syntax

!listing mfem/complex/mixed_sesquilinear.i block=ICs

!syntax parameters /ICs/MFEMComplexScalarIC

!syntax inputs /ICs/MFEMComplexScalarIC

!syntax children /ICs/MFEMComplexScalarIC

!if-end!

!else
!include mfem/mfem_warning.md
