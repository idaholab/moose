# MFEMCurlAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMCurlAux

## Overview

AuxKernel for calculating the curl of an $H(\mathrm{curl})$ conforming source variable defined on a 3D Nédélec finite element
space and storing it in an $H(\mathrm{div})$ conforming result variable defined on a Raviart-Thomas finite element space.

The result may be scaled by an optional (global) scalar factor.

!equation
\vec v =  \lambda \vec\nabla \times \vec u

where $\vec u \in H(\mathrm{curl})$, $\vec v \in H(\mathrm{div})$ and $\lambda$ is a scalar constant.

## Example Input File Syntax

!listing curlcurl.i

!syntax parameters /AuxKernels/MFEMCurlAux

!syntax inputs /AuxKernels/MFEMCurlAux

!syntax children /AuxKernels/MFEMCurlAux

!else
!include mfem/mfem_warning.md
