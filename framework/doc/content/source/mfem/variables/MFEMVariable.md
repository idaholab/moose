# MFEMVariable

## Summary

!syntax description /Variables/MFEMVariable

## Overview

`MFEMVariable` defines a finite element variable (`mfem::ParGridFunction`) with respect to a finite
element space created on the problem's mesh. Multiple `MFEMVariable` objects can be created with
respect to the same `MFEMFESpace`.

## Example Input File Syntax

Preferentially, users should create an  `MFEMVariable` with respect to an `MFEMFESpace`:

!listing test/tests/kernels/diffusion.i block=Problem FESpaces Variables

However, if a user creates a MOOSE variable in an `MFEMProblem`, then Platypus should automatically
create the corresponding `MFEMFESpace` and `MFEMVariable` for that type:

!listing test/tests/variables/mfem_variables_from_moose.i block=Problem Variables

!syntax parameters /Variables/MFEMVariable

!syntax inputs /Variables/MFEMVariable

!syntax children /Variables/MFEMVariable
