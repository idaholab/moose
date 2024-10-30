# MFEMVectorFunctionCoefficient

## Summary

!syntax description /VectorCoefficients/MFEMVectorFunctionCoefficient

## Overview

Class defining `mfem::VectorFunctionCoefficient` objects to be added to MFEM problems. The
`mfem::VectorFunctionCoefficient` takes a named MOOSE vector function of time and space in its constructor.

## Example Input File Syntax

!listing test/tests/kernels/curlcurl.i block=Problem Functions VectorCoefficients

!syntax parameters /VectorCoefficients/MFEMVectorFunctionCoefficient

!syntax inputs /VectorCoefficients/MFEMVectorFunctionCoefficient

!syntax children /VectorCoefficients/MFEMVectorFunctionCoefficient
