# MFEMMatrixCoefficientFunction

!if! function=hasCapability('mfem')

## Overview

`MFEMMatrixCoefficientFunction` declares a named
[MFEM matrix coefficient](https://mfem.org/coefficient/#matrix-coefficients)
assembled from scalar coefficient names and numeric literals.

The required [!param](/Functions/MFEMMatrixCoefficientFunction/matrix) uses
semicolons to separate rows and whitespace to separate entries. Its dimension is
inferred from the entries; the matrix must be nonempty and square, with no empty
or uneven rows.

Each matrix element is resolved as a scalar MFEM coefficient. Supported scalar
sources include [ParsedFunction](MooseParsedFunction.md) objects,
[MFEMParsedFunction](MFEMParsedFunction.md) objects, scalar material
coefficients, and numeric literals. Inline expressions for matrix elements are
not supported.

## Example input syntax

This small problem declares a non-symmetric matrix, mixing names with literals.

!listing test/tests/mfem/functions/matrix_coefficient_function.i block=Functions

This embedded-surface example uses a spatially varying 3x3 matrix, assembled
from scalar functions and constant literals.

!listing test/tests/mfem/kernels/variable_anisotropic_diffusion.i block=Functions

!syntax parameters /Functions/MFEMMatrixCoefficientFunction

!syntax inputs /Functions/MFEMMatrixCoefficientFunction

!syntax children /Functions/MFEMMatrixCoefficientFunction

!if-end!

!else
!include mfem/mfem_warning.md
