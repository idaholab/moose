# MFEMVariable

!if! function=hasCapability('mfem')

## Overview

`MFEMVariable` defines a finite element variable (`mfem::ParGridFunction`) with respect to a finite
element space created on the problem's mesh. Multiple `MFEMVariable` objects can be created with
respect to the same `MFEMFESpace`.

When creating an MFEMVariable, a number of auxiliary `mfem::Coefficient` and
`mfem::VectorCoefficient` classes are declared for ease-of-use when referencing this variable in
functions, kernels, and postprocessors.

Specifically for scalar variables, scalar coefficients are added representing:

- the variable itself, with the same name as the variable name
- the magnitude of the gradient of the variable (if available), with the suffix `_grad_mag`

and vector coefficients are added representing

- the gradient of the variable (if available), with the suffix `_grad`.

For vector variables, scalar coefficients are added representing:

- the magnitude of the variable, with the suffix `_mag`
- the divergence of the variable (if available), with the suffix `_div`
- the magnitude of the curl of the variable (if available), with the suffix `_curl_mag`

and vector coefficients are added representing

- the variable itself, with the same name as the variable name
- the curl of the variable (if available), with the suffix `_curl`.

## Example Input File Syntax

Preferentially, users should create an  `MFEMVariable` with respect to an `MFEMFESpace`:

!listing test/tests/mfem/kernels/diffusion.i block=Problem FESpaces Variables

However, if a user creates a MOOSE variable in an `MFEMProblem`, then MOOSE should automatically
create the corresponding `MFEMFESpace` and `MFEMVariable` for that type:

!listing test/tests/mfem/variables/mfem_variables_from_moose.i block=Problem Variables

!syntax parameters /Variables/MFEMVariable

!syntax inputs /Variables/MFEMVariable

!syntax children /Variables/MFEMVariable

!if-end!

!else
!include mfem/mfem_warning.md
