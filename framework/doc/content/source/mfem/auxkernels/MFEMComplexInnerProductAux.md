# MFEMComplexInnerProductAux

!if! function=hasCapability('mfem')

## Overview

AuxKernel for calculating the inner product of two complex vector fields and projecting onto an L2 finite element space mfem complex auxvariable.

!equation
s u\cdot v*, \,\,\,

where $u$ and $v$ are the two fields and $s$ is an optional complex scaling. Here, * represents complex conjugation.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexInnerProductAux

!syntax inputs /AuxKernels/MFEMComplexInnerProductAux

!syntax children /AuxKernels/MFEMComplexInnerProductAux

!if-end!

!else
!include mfem/mfem_warning.md
