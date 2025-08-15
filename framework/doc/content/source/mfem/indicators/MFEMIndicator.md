# MFEMIndicator

!if! function=hasCapability('mfem')

## Summary

Virtual base class for element based error estimators.

## Overview

`MFEMIndicator` is responsible for building an `mfem::ErrorEstimator` object using a variable and its kernel.
The error estimator is then passed on to an [MFEMRefinementMarker](source/mfem/markers/MFEMRefinementMarker.md)
object.

Broadly speaking, the `mfem::ErrorEstimator` object looks at one variable and its kernel (after a solve step)
and determines which, if any, regions of the mesh need to be refined further. These recommendations are then
passed on to the `MFEMRefinementMarker`, which does the actual refinement.

This class serves as an abstract base class, and does no actual implementation. 

To keep the naming consistent with moose, we refer to this class as an Indicator.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion_amr.i block=Adaptivity


!if-end!

!else
!include mfem/mfem_warning.md
