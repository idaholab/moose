# MFEMFunctionCoefficient

## Summary

!syntax description /Coefficients/MFEMFunctionCoefficient

## Overview

Class defining `mfem::FunctionCoefficient` objects to be added to MFEM problems. The
`mfem::FunctionCoefficient` takes a named MOOSE function of time and space in its constructor.

## Example Input File Syntax

!listing test/tests/kernels/heatconduction.i block=Problem Functions Coefficients

!syntax parameters /Coefficients/MFEMFunctionCoefficient

!syntax inputs /Coefficients/MFEMFunctionCoefficient

!syntax children /Coefficients/MFEMFunctionCoefficient
