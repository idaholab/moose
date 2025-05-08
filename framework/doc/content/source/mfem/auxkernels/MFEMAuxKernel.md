# MFEMAuxKernel

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM auxkernels used to evaluate auxiliary variables to the main solve.

## Overview

MFEM auxkernels are responsible for updating auxiliary variables in the system, during pre- or
post-processing steps.

An `MFEMAuxKernel` is derived from `MFEMGeneralUserObject`, and thus the order of their execution
can be controlled similar to other MOOSE UserObjects using the `execution_order_group` input
parameter.

`MFEMAuxKernel` is a purely virtual base class. Derived classes should override the `execute`
 method to update the `_result_var` during execution.

!if-end!

!else
!include mfem/mfem_warning.md
