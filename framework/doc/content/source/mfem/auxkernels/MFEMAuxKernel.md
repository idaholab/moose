# MFEMAuxKernel

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM auxkernels used to evaluate real auxiliary variables to the main solve.

## Overview

MFEM auxkernels are responsible for updating real auxiliary variables in the system during MFEM
execution passes.

An `MFEMAuxKernel` is derived from `MFEMExecutedObject`. Its ordering relative to other MFEM
executed objects is determined automatically from detected data dependencies rather than by manual
execution groups.

`MFEMAuxKernel` is a base class. Derived classes should override the `execute`
 method to update the `_result_var` during execution.

!if-end!

!else
!include mfem/mfem_warning.md
