# MFEMComplexIntegratedBC

!if! function=hasCapability('mfem')

## Summary

Base class for objects applying complex-valued integrated boundary conditions to an MFEM FE problem.

## Overview

`MFEMComplexIntegratedBC` applies boundary integrator(s) to the weak form equation that is labeled
according to the test variable name returned from `getTestVariableName()`, similar to
[`MFEMComplexKernel`](source/mfem/kernels/MFEMKernel.md) for domain integrators.

Similarly to how [`MFEMComplexKernel`](source/mfem/kernels/MFEMComplexKernel.md) works, `MFEMComplexIntegratedBC`
is a container for two `MFEMIntegratedBC` objects, one representing the real part of the integrated BC, and the other
representing the imaginary part. These two can be set up using the `RealComponent` and `ImagComponent` sub-blocks on the script.
The two integrators need not be the same, but they do need to be applied to the same variable.

!syntax parameters /BCs/MFEMComplexIntegratedBC

!syntax inputs /BCs/MFEMComplexIntegratedBC

!syntax children /BCs/MFEMComplexIntegratedBC


!if-end!

!else
!include mfem/mfem_warning.md
