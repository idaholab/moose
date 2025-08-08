# AddMFEMComplexKernelComponentAction

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/AddMFEMComplexKernelComponentAction

## Overview

Action called to add a real or imaginary component of an [MFEMComplexKernel](source/mfem/kernels/MFEMComplexKernel.md).
Each of these is included as an `AuxKernel`, to be retrieved later when the `MFEMComplexKernel` object is created.

## Example Input File Syntax

!listing test/tests/mfem/kernels/complex.i block=Kernels

!syntax parameters /Kernels/AddMFEMComplexKernelComponentAction

!if-end!

!else
!include mfem/mfem_warning.md
