# MFEMCoordinateTransformations

!if! function=hasCapability('mfem')

##Summary 

Function object for declaring coordinate dependent MFEM scalar coefficients. 

## Overview

`MFEMCoordinateTransformations` is a function object created under the
top-level `[Functions]` block. Rather than introducing a separate
coordinate-system block, this object provides a lightweight way to
declare coordinate-dependent MFEM scalar coefficients into the
`CoefficientManager`, where they can then be used through the normal
MFEM scalar coefficient path.

This allows the coordinate coefficients to be used in objects such as
`MFEMGenericFunctorMaterial` property definitions or MFEM kernels.

Currently this object supports only the cylindrical/axisymmetric
coordinate type

- `coord_type = RZ`

## Built-in Coefficients for `coord_type = RZ`

- `r`, the radial coefficient, built from MFEM’s
  [`mfem::CylindricalRadialCoefficient`](https://docs.mfem.org/html/coefficient_8hpp_source.html)
- `inv_r`, the regularized inverse radial coefficient
- `two_pi_r` the full cylindrical measure factor {2\pi\text{r}}

The `two_pi_r` coefficient is constructed using [`mfem::TransformedCoefficient`](https://docs.mfem.org/html/classmfem_1_1TransformedCoefficient.html).

The `inv_r` coefficient is regularized as
!equation
{\text{inv\_r} = \sqrt\frac{1}{r^2 + \varepsilon^2}}

where $\varepsilon$ is specified by the `inv_r_eps` input parameter.


## Example Input File Syntax

!listing test/tests/mfem/functions/cylindrical_coefficients.i block=Functions

## Example Use in Materials

!listing test/tests/mfem/functions/cylindrical_coefficients.i block=FunctorMaterials

!syntax parameters /Functions/MFEMCoordinateTransformations

!syntax inputs /Functions/MFEMCoordinateTransformations

!syntax children /Functions/MFEMCoordinateTransformations

!if-end!

!else
!include mfem/mfem_warning.md