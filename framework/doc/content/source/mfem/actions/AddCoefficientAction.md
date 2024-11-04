# AddCoefficientAction

## Summary

!syntax description /Coefficients/AddCoefficientAction

## Overview

Action called to add an MFEM coefficient to the problem, parsing content inside a `Coefficients`
block in the user input. Only has an effect if the `Problem` type is set to `MFEMProblem`.

## Example Input File Syntax

!listing test/tests/kernels/heatconduction.i block=Problem Functions Coefficients

!syntax parameters /Coefficients/AddCoefficientAction
