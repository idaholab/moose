# AddMFEMComplexKernelComponentAction

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/AddMFEMComplexKernelComponentAction

## Overview

Action called to add a real or imaginary component of an [MFEMComplexKernel](source/mfem/kernels/MFEMComplexKernel.md), each in the form of a separate [MFEMKernel](source/mfem/kernels/MFEMKernel.md) user object.

## Example Input File Syntax

!listing test/tests/mfem/kernels/complex.i block=Kernels

!syntax parameters /Kernels/AddMFEMComplexKernelComponentAction

!if-end!

!else
!include mfem/mfem_warning.md
