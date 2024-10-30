# MFEMCoefficient

## Summary

!syntax description /Coefficients/MFEMCoefficient

## Overview

Base class for `mfem::Coefficient` objects to be added to MFEM problems. Derived classes should
override the `getCoefficient` method to return the `mfem::Coefficient` of the desired type.

## Example Input File Syntax

!listing test/tests/kernels/heatconduction.i block=Problem Functions Coefficients

!syntax parameters /Coefficients/MFEMCoefficient

!syntax inputs /Coefficients/MFEMCoefficient

!syntax children /Coefficients/MFEMCoefficient
