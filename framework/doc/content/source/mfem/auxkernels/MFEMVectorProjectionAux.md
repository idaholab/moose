# MFEMVectorProjectionAux

!if! function=hasCapability('mfem')

## Overview

AuxKernel for projecting a vector coefficient onto a vector auxiliary variable
in, e.g., $H(\mathrm{curl})$ or $H(\mathrm{div})$.

Note that [MFEMGradAux.md] and [MFEMCurlAux.md] should be preferred over projection of vector
coefficients representing the gradient of scalar variables and curl of vector variables respectively
where available for performance reasons.

## Example Input File Syntax

!listing mfem/auxkernels/projection.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMVectorProjectionAux

!syntax inputs /AuxKernels/MFEMVectorProjectionAux

!syntax children /AuxKernels/MFEMVectorProjectionAux

!if-end!

!else
!include mfem/mfem_warning.md
