# MFEMComplexVariable

!if! function=hasCapability('mfem')

## Overview

`MFEMComplexVariable` defines a complex-valued finite element variable (`mfem::ParComplexGridFunction`) with respect to a finite
element space created on the problem's mesh. Multiple `MFEMComplexVariable` objects can be created with
respect to the same `MFEMFESpace`.

## Example Input File Syntax

Preferentially, users should create an  `MFEMComplexVariable` with respect to an `MFEMFESpace`:

!listing test/tests/mfem/complex/complex.i block=Problem FESpaces Variables

!syntax parameters /Variables/MFEMComplexVariable

!syntax inputs /Variables/MFEMComplexVariable

!syntax children /Variables/MFEMComplexVariable

!if-end!

!else
!include mfem/mfem_warning.md
