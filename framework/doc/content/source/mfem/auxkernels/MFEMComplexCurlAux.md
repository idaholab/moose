# MFEMComplexCurlAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMComplexCurlAux

## Overview

AuxKernel for calculating the curl of a complex $H(\mathrm{curl})$ conforming source variable defined on a 3D Nédélec finite element
space and storing it in a complex $H(\mathrm{div})$ conforming result variable defined on a Raviart-Thomas finite element space.

The result may be scaled by an optional (global) complex scalar factor.

!equation
\vec v =  \lambda \vec\nabla \times \vec u

where $\vec u \in H(\mathrm{curl})$, $\vec v \in H(\mathrm{div})$ and $\lambda$ is a complex scalar constant.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexCurlAux

!syntax inputs /AuxKernels/MFEMComplexCurlAux

!syntax children /AuxKernels/MFEMComplexCurlAux

!if-end!

!else
!include mfem/mfem_warning.md
