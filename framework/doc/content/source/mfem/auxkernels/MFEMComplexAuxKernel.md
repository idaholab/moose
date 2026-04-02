# MFEMComplexAuxKernel

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM auxkernels used to evaluate complex auxiliary variables to the main solve.

## Overview

Complex MFEM auxkernels are responsible for updating complex auxiliary variables in the system
during MFEM execution passes.

An `MFEMComplexAuxKernel` is derived from `MFEMExecutedObject`. Its ordering relative to other MFEM
executed objects is determined automatically from detected data dependencies rather than by manual
execution groups.

`MFEMComplexAuxKernel` is a base class. Derived classes should override the `execute`
 method to update the `_result_var` during execution.

!if-end!

!else
!include mfem/mfem_warning.md
