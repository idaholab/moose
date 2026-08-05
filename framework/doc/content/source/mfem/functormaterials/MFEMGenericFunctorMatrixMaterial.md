# MFEMGenericFunctorMatrixMaterial

!if! function=hasCapability('mfem')

## Overview

`MFEMGenericFunctorMatrixMaterial` defines one or more matrix material properties with values
obtained from coefficients on one or more subdomains of the mesh, given by the [!param](/FunctorMaterials/MFEMGenericFunctorMatrixMaterial/block) parameter
if provided, or applied to the entire mesh if missing. The matrix material properties are named
according to members in the [!param](/FunctorMaterials/MFEMGenericFunctorMatrixMaterial/prop_names) parameter, with respective coefficients used to get property
values given by the members of
[!param](/FunctorMaterials/MFEMGenericFunctorMatrixMaterial/prop_values). The
coefficients in
[!param](/FunctorMaterials/MFEMGenericFunctorMatrixMaterial/prop_names)
must be matrix-valued. Numeric constant matrix values can also be
specified, but must be enclosed in curly braces to mark the start and
end of the matrix, with entries given row by row and rows separated by
semicolons, e.g. `{1. 2.; 3. 4.}`.

## Example Input File Syntax

!listing test/tests/mfem/kernels/maxwell_anisotropic_eigenproblem.i block=/FunctorMaterials

!syntax parameters /FunctorMaterials/MFEMGenericFunctorMatrixMaterial

!syntax inputs /FunctorMaterials/MFEMGenericFunctorMatrixMaterial

!syntax children /FunctorMaterials/MFEMGenericFunctorMatrixMaterial

!if-end!

!else
!include mfem/mfem_warning.md
