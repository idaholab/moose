# MFEMComplexScalarBoundaryIC

!if! function=hasCapability('mfem')

## Overview

`MFEMComplexScalarBoundaryIC` is used to set initial conditions of complex scalar valued MFEM variables from two
`mfem::Coefficient`s representing its real and imaginary parts. The initial condition will be applied on all boundaries specified by the user
in the mesh.

## Example Input File Syntax

!listing mfem/submeshes/cut_closed_coil.i block=ICs

!syntax parameters /ICs/MFEMComplexScalarBoundaryIC

!syntax inputs /ICs/MFEMComplexScalarBoundaryIC

!syntax children /ICs/MFEMComplexScalarBoundaryIC

!if-end!

!else
!include mfem/mfem_warning.md
