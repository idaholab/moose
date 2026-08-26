# AddMFEMFESpaceAction

!if! function=hasCapability('mfem')

## Overview

Action called to add an MFEM coordinate dependent built-in coefficients, parsed inside a [`MFEMCoordinateCofficients`](source/mfem/coordinates/MFEMCoordinateCoefficients.md) block in the user input. These are created under the top level [`Coordinates`] block and are responsible for constructing and exposing the coordinate system depedent scalar coefficients to the MFEM `CoefficientManager`.

This action only has an effect if the `Problem` type is set to [`MFEMProblem`](source/mfem/problem/MFEMProblem.md).

## Example Input File Syntax

!listing test/tests/mfem/coordinates/cylindrical_coefficients.i block=Problem Coordinates

!syntax parameters /Coordinates/AddMFEMFECoordinateCoefficientsAction

!if-end!

!else
!include mfem/mfem_warning.md
