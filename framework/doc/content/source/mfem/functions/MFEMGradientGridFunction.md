# MFEMGradientGridFunction

!if! function=hasCapability('mfem')

## Summary

This is class which can be use as vector cofficients to MFEM problems.

## Overview

`MFEMGradientGridFunction` is intended to allow the specification of `mfem::VectorCoefficient` object to add to the MFEM problem in a manner consistent with the standard MOOSE Materials system.

!if-end!

!else
!include mfem/mfem_warning.md
