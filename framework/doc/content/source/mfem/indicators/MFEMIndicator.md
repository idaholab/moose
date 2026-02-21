# MFEMIndicator

!if! function=hasCapability('mfem')

## Summary

Virtual base class for element-based error estimators.

## Overview

`MFEMIndicator` is responsible for building an `mfem::ErrorEstimator` object using a variable and one of its kernels.
The error estimator is then passed on to an [MFEMRefinementMarker.md] object.

Broadly speaking, the `mfem::ErrorEstimator` object looks, after a solve step, at one variable and one of its kernels as chosen by the user
and determines which, if any, regions of the mesh need to be refined further. These recommendations are then
passed on to the `MFEMRefinementMarker`, which does the actual refinement.

This class serves as an abstract base class, and does no actual implementation. 

To keep the naming consistent with similar classes in MOOSE, we refer to this class as an Indicator.

!if-end!

!else
!include mfem/mfem_warning.md
