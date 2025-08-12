# MFEMVectorPostprocessor

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM vectorpostprocessors used to evaluate an array of values.

## Overview

MFEM vectorpostprocessors calculate an array of values from the (aux)variables,
typically after each timestep.

An `MFEMVectorPostprocessor` is derived from `MFEMGeneralUserObject`.
Therefore, the order of their execution can be controlled similar to other
MOOSE UserObjects using the `execution_order_group` input parameter, e.g., to
require the execution of a vectorpostprocessor computing on an [AuxVariable.md]
strictly after the execution of the [MFEMAuxKernel.md] computing the variable
field itself.

`MFEMVectorPostprocessor` is a virtual base class. Derived classes should use
the `VectorPostprocessor::declareVector` method to get a reference to a vector
which will be output.

!if-end!

!else
!include mfem/mfem_warning.md
