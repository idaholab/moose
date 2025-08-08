# AddMFEMComplexBCComponentAction

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/AddMFEMComplexBCComponentAction

## Overview

Action called to add a real or imaginary component of an [MFEMComplexIntegratedBC](source/mfem/bcs/MFEMComplexIntegratedBC.md).
Each of these is included as an `AuxBoundaryCondition`, to be retrieved later when the `MFEMComplexIntegratedBC` object is created.

!syntax parameters /BCs/AddMFEMComplexBCComponentAction

!if-end!

!else
!include mfem/mfem_warning.md
