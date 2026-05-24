# MFEMScalarProjectionAux

!if! function=hasCapability('mfem')

## Overview

AuxKernel for projecting a scalar coefficient onto a scalar auxiliary variable
in, e.g., $H^1$ or $L^2$.

Note that [MFEMDivAux.md] should be preferred over projection of coefficients representing variable
divergences where available for performance reasons.

## Example Input File Syntax

!listing mfem/auxkernels/projection.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMScalarProjectionAux

!syntax inputs /AuxKernels/MFEMScalarProjectionAux

!syntax children /AuxKernels/MFEMScalarProjectionAux

!if-end!

!else
!include mfem/mfem_warning.md
