# MFEMComplexExteriorProductAux

!if! function=hasCapability('mfem')

## Overview

AuxKernel for calculating the exterior product of two complex vector fields and projecting
the result onto an MFEM auxiliary variable in an L2 complex vector finite element space.

!equation
s \vec u \wedge \vec v*

where $\vec u$ and $\vec v$ are the two complex vector fields and $s$ is an optional
complex scaling coefficient.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexExteriorProductAux

!syntax inputs /AuxKernels/MFEMComplexExteriorProductAux

!syntax children /AuxKernels/MFEMComplexExteriorProductAux

!if-end!

!else
!include mfem/mfem_warning.md
