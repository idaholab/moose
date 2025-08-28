# MFEMGradAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMGradAux

## Overview

AuxKernel for calculating the gradient of a scalar $H^1$ conforming source variable and storing it in
a scalar elemental result variable defined on an $H(\mathrm{curl})$ conforming Nédélec finite element space.

The result may be scaled by an optional (global) scalar factor.

!equation
\vec v = \lambda \vec\nabla u

where $u \in H^1$, $\vec v \in H(\mathrm{curl})$ and $\lambda$ is a scalar constant.

## Example Input File Syntax

!listing mfem/kernels/diffusion.i block=AuxKernels

!listing kernels/irrotational.i block=AuxKernels

!syntax parameters /AuxKernels/MFEMGradAux

!syntax inputs /AuxKernels/MFEMGradAux

!syntax children /AuxKernels/MFEMGradAux

!if-end!

!else
!include mfem/mfem_warning.md
