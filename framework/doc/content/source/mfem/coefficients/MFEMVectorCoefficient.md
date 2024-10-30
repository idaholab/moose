# MFEMVectorCoefficient

## Summary

!syntax description /VectorCoefficients/MFEMVectorCoefficient

## Overview

Base class for `mfem::VectorCoefficient` objects to be added to MFEM problems. Derived classes should
override the `getVectorCoefficient` method to return the `mfem::Coefficient` of the desired type.

## Example Input File Syntax

!listing test/tests/kernels/curlcurl.i block=Problem Functions VectorCoefficients

!syntax parameters /VectorCoefficients/MFEMVectorCoefficient

!syntax inputs /VectorCoefficients/MFEMVectorCoefficient

!syntax children /VectorCoefficients/MFEMVectorCoefficient
