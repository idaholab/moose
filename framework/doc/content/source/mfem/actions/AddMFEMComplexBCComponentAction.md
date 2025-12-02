# AddMFEMComplexBCComponentAction

!if! function=hasCapability('mfem')

## Overview

Action called to add a real or imaginary component of an [MFEMComplexIntegratedBC](source/mfem/bcs/MFEMComplexIntegratedBC.md), each in the form of a separate [MFEMIntegratedBC](source/mfem/bcs/MFEMIntegratedBC.md) user object.

!syntax parameters /BCs/AddMFEMComplexBCComponentAction

!if-end!

!else
!include mfem/mfem_warning.md
