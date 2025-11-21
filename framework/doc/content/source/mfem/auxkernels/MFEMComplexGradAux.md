# MFEMComplexGradAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMComplexGradAux

## Overview

AuxKernel for calculating the gradient of a complex scalar $H^1$ conforming source variable and storing it in
a complex scalar elemental result variable defined on an $H(\mathrm{curl})$ conforming Nédélec finite element space.

The result may be scaled by an optional (global) complex scalar factor.

!equation
\vec v = \lambda \vec\nabla u

where $u \in H^1$, $\vec v \in H(\mathrm{curl})$ and $\lambda$ is a complex scalar constant.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexGradAux

!syntax inputs /AuxKernels/MFEMComplexGradAux

!syntax children /AuxKernels/MFEMComplexGradAux

!if-end!

!else
!include mfem/mfem_warning.md
