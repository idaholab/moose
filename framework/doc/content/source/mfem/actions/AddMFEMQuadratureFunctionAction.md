# AddMFEMQuadratureFunctionAction

!if! function=hasCapability('mfem')

## Overview

Action called to add an MFEM quadrature function coefficient to the problem, parsing content
inside a [`QuadratureFunctions`](source/mfem/functions/MFEMScalarQuadratureFunction.md) block in
the user input. Only has an effect if the `Problem` type is set to [MFEMProblem.md].

## Example Input File Syntax

!listing test/tests/mfem/functions/quadrature_function_source.i block=Problem QuadratureFunctions

!syntax parameters /QuadratureFunctions/AddMFEMQuadratureFunctionAction

!if-end!

!else
!include mfem/mfem_warning.md
