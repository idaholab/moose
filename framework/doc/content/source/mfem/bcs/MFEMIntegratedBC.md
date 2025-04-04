# MFEMIntegratedBC

## Summary

Base class for objects applying integrated boundary conditions to an MFEM FE problem.

## Overview

Classes deriving from `MFEMIntegratedBC` represent boundary conditions which apply one or more
boundary integrators to the weak form on the set of user-specified boundaries.

`MFEMIntegratedBC` applies boundary integrator(s) to the weak form equation that is labeled
according to the test variable name returned from `getTestVariableName()`, similar to
[`MFEMKernel`](source/mfem/kernels/MFEMKernel.md) for domain integrators.
