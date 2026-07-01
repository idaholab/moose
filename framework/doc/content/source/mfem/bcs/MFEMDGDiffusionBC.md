# MFEMDGDiffusionBC

!if! function=hasCapability('mfem')

## Overview

Adds a boundary face integrator to the bilinear form. The DG parameters sigma and kappa can be set, but both have default values of
-1.0 and (order+1)^2 respectively.  A value of $\sigma = -1$ corresponds to a symmetric interior penalty DG method.

## Example Input File Syntax

!listing test/tests/mfem/kernels/dg_diffusion.i block=BCs

!syntax parameters /BCs/MFEMDGDiffusionBC

!syntax inputs /BCs/MFEMDGDiffusionBC

!if-end!

!else
!include mfem/mfem_warning.md
