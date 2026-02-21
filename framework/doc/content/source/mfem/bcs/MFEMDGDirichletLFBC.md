# MFEMDGDirichletLFBC

!if! function=hasCapability('mfem')

## Overview

Adds a boundary face integrator to the linear form. The DG parameters sigma and kappa can be set, but both have default values of
-1.0 and (order+1)^2 respectively.

## Example Input File Syntax

!listing test/tests/mfem/kernels/dg_diffusion.i block=BCs

!syntax parameters /BCs/MFEMDGDirichletLFBC

!syntax inputs /BCs/MFEMDGDirichletLFBC

!if-end!

!else
!include mfem/mfem_warning.md
