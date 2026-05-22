# MFEMComplexVariable

!if! function=hasCapability('mfem')

## Overview

`MFEMComplexVariable` defines a complex-valued finite element variable (`mfem::ParComplexGridFunction`) with respect to a finite
element space created on the problem's mesh. Multiple `MFEMComplexVariable` objects can be created with
respect to the same `MFEMFESpace`.

When creating an `MFEMComplexVariable`, a number of auxiliary `mfem::Coefficient` and
`mfem::VectorCoefficient` classes are declared for ease-of-use when referencing this variable in
functions, kernels, and postprocessors. Coefficient names follow the convention described in
[MFEMVariable.md], with the component label (`_real` or `_imag`) included as the first suffix, with
suffixes relating to derivatives (`_grad`, `_div`, or `_curl`) and/or any suffix taking the vector
magnitude (`_mag`) following thereafter in that order.

## Example Input File Syntax

Preferentially, users should create an  `MFEMComplexVariable` with respect to an `MFEMFESpace`:

!listing test/tests/mfem/complex/complex.i block=Problem FESpaces Variables

!syntax parameters /Variables/MFEMComplexVariable

!syntax inputs /Variables/MFEMComplexVariable

!syntax children /Variables/MFEMComplexVariable

!if-end!

!else
!include mfem/mfem_warning.md
