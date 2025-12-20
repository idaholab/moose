# MFEMComplexInnerProductAux

!if! function=hasCapability('mfem')

## Overview

AuxKernel for calculating the inner product of two complex vector fields and projecting
the result onto an MFEM complex auxiliary variable in an L2 finite element space.

!equation
s \vec u \cdot \vec v*

where $\vec u$ and $\vec v$ are the two complex vector fields and $s$ is an optional
complex scaling coefficient.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexInnerProductAux

!syntax inputs /AuxKernels/MFEMComplexInnerProductAux

!syntax children /AuxKernels/MFEMComplexInnerProductAux

!if-end!

!else
!include mfem/mfem_warning.md
