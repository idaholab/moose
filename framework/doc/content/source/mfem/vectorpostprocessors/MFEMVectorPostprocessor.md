# MFEMVectorPostprocessor

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM vectorpostprocessors used to evaluate an array of values.

## Overview

MFEM vectorpostprocessors calculate an array of values from the (aux)variables, typically after
each timestep.

An `MFEMVectorPostprocessor` is derived from `MFEMExecutedObject`. Its ordering relative to other
MFEM executed objects is determined automatically from detected data dependencies instead of manual
execution groups.

`MFEMVectorPostprocessor` is a virtual base class. Derived classes should use
the `VectorPostprocessor::declareVector` method to get a reference to a vector
which will be output.

!if-end!

!else
!include mfem/mfem_warning.md
