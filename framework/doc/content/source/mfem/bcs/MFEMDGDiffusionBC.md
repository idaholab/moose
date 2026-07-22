# MFEMDGDiffusionBC

!if! function=hasCapability('mfem')

## Overview

Adds a boundary face integrator to the linear/bilinear forms.
`createBFIntegrator()` returns a `mfem::DGDiffusionIntegrator` object ([see mfem docs for more info](https://docs.mfem.org/html/classmfem_1_1DGDiffusionIntegrator.html)).
`createLFIntegrator()` returns a `mfem::DGDirichletLFIntegrator` object ([see mfem docs for more info](https://docs.mfem.org/html/classmfem_1_1DGDirichletLFIntegrator.html)).
The DG parameters $\sigma$ and $\kappa$ can be set, but both have default values of
$\sigma = -1$ and $\kappa = (o+1)^2$, respectively. $o$ is the order of the finite element space of the variable this BC applies to.
A value of $\sigma = -1$ corresponds to a symmetric interior penalty DG method.

## Example Input File Syntax

!listing test/tests/mfem/kernels/dg_diffusion.i block=BCs

!syntax parameters /BCs/MFEMDGDiffusionBC

!syntax children /BCs/MFEMDGDiffusionBC

!if-end!

!else
!include mfem/mfem_warning.md
