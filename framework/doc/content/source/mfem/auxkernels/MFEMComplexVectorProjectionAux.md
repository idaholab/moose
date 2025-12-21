# MFEMComplexVectorProjectionAux

!if! function=hasCapability('mfem')

## Overview

AuxKernel for projecting a pair of vector coefficients onto a complex vector auxiliary variable
in, e.g., $H(\mathrm{curl})$ or $H(\mathrm{div})$.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexVectorProjectionAux

!syntax inputs /AuxKernels/MFEMComplexVectorProjectionAux

!syntax children /AuxKernels/MFEMComplexVectorProjectionAux

!if-end!

!else
!include mfem/mfem_warning.md
