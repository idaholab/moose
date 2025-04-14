# MFEMPostprocessor

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM postprocessors used to evaluate a single scalar.

## Overview

MFEM postprocessors calculate scalar quantities from the
(aux)variables, typically after each timestep.

An `MFEMPostprocessor` is derived from `MFEMPostprocessor` and thus
`MFEMGeneralUserObject`. Therefore, the order of their execution can
be controlled similar to other MOOSE UserObjects using the
`execution_order_group` input parameter.

`MFEMPostprocessor` is a purely virtual base class. Derived classes
should override the `execute` and `getValue` methods.

!if-end!

!else
!include mfem/mfem_warning.md
