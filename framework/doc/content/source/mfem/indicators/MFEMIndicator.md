# MFEMIndicator

!if! function=hasCapability('mfem')

## Summary

Virtual base class for element based error estimators.

## Overview

MFEM Estimators are resonsible for providing Error Estimators to [Refiners](source/mfem/refiners/MFEMThresholdMarker.md). 
The `mfem::ErrorEstimator` object is constructed using a variable and a kernel which applies to that variable.


`MFEMIndicator` is an abstract base class and should correctly implement
the `createEstimator()` method.

!if-end!

!else
!include mfem/mfem_warning.md
