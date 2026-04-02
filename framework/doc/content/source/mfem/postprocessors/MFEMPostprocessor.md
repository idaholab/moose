# MFEMPostprocessor

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM postprocessors used to evaluate a single scalar.

## Overview

MFEM postprocessors calculate scalar quantities from the (aux)variables, typically after each
timestep.

An `MFEMPostprocessor` is derived from `MFEMExecutedObject`. Its ordering relative to MFEM initial
conditions, aux kernels, transfers, and other MFEM postprocessors is determined automatically from
detected data dependencies instead of manual execution groups.

`MFEMPostprocessor` is a purely virtual base class. Derived classes
should override the `execute` and `getValue` methods.

!if-end!

!else
!include mfem/mfem_warning.md
