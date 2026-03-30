# MFEMEigenvaluesPostprocessor

!if! function=hasCapability('mfem')

## Overview

Retrieves the computed eigenvalues from an MFEM eigensolver after each solve and exports them as a
vector quantity. This postprocessor must be used in conjunction with an
[MFEMEigenproblem](problem/MFEMEigenproblem.md) problem type; using it with any other problem type
will result in an error.

The number of eigenvalues reported corresponds to the `num_modes` parameter set on
`MFEMEigenproblem`. The eigenvalues are stored in ascending order and are typically written to a
CSV output file for further analysis.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion_eigenproblem.i block=VectorPostprocessors Outputs

!syntax parameters /VectorPostprocessors/MFEMEigenvaluesPostprocessor

!syntax inputs /VectorPostprocessors/MFEMEigenvaluesPostprocessor

!syntax children /VectorPostprocessors/MFEMEigenvaluesPostprocessor

!if-end!

!else
!include mfem/mfem_warning.md
