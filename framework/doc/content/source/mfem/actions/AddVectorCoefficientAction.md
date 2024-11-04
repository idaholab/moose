# AddVectorCoefficientAction

## Summary

!syntax description /VectorCoefficients/AddVectorCoefficientAction

## Overview

Action called to add an MFEM vector coefficient to the problem, parsing content inside a `VectorCoefficients`
block in the user input. Only has an effect if the `Problem` type is set to `MFEMProblem`.

## Example Input File Syntax

!listing test/tests/kernels/curlcurl.i block=Problem Functions Coefficients

!syntax parameters /VectorCoefficients/AddVectorCoefficientAction
