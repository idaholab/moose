# AddMFEMCoordinateCoefficientsAction

!if! function=hasCapability('mfem')

## Overview

Action called to add MFEM coordinate dependent built-in coefficients, parsed inside a [`MFEMCoordinateCoefficients`](source/mfem/coordinates/MFEMCoordinateCoefficients.md) block. These are created under the top level `CoordinateSystem` block and are responsible for constructing and exposing the coordinate system dependent scalar coefficients to the MFEM `CoefficientManager`.

This action only has an effect if the `Problem` type is set to [`MFEMProblem`](source/mfem/problem/MFEMProblem.md).

## Example Input File Syntax

!listing test/tests/mfem/coordinates/cylindrical_coefficients.i block=CoordinateSystem

!syntax parameters /Coordinates/AddMFEMCoordinateCoefficientsAction

!if-end!

!else
!include mfem/mfem_warning.md
