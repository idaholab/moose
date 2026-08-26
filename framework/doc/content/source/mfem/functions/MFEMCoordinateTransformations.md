# MFEMCoordinateTransformations

!if! function=hasCapability('mfem')

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

## Built-in Coefficients 

`coord_type = RZ`

- `<name>_r`, the radial coefficient, built from MFEM's
  [`mfem::CylindricalRadialCoefficient`](https://docs.mfem.org/html/classmfem_1_1CylindricalRadialCoefficient.html)

- `<name>_inv_r`, the inverse radial coefficient, regularized as $\text{inv\_r} = \sqrt{\frac{1}{r^2 + \varepsilon^2}}$,
  where $\varepsilon$ is specified by the `inv_r_eps` input parameter, constructed from `<name>_r` using
  [`mfem::TransformedCoefficient`](https://docs.mfem.org/html/classmfem_1_1TransformedCoefficient.html)

- `<name>_two_pi_r`, the full cylindrical measure factor $2 \pi r$, constructed from `<name>_r` using
  [`mfem::ProductCoefficient`](https://docs.mfem.org/html/classmfem_1_1ProductCoefficient.html)

- `<name>_p`, the azimuthal angle, built from MFEM's
  [`mfem::CylindricalAzimuthalCoefficient`](https://docs.mfem.org/html/classmfem_1_1CylindricalAzimuthalCoefficient.html)

- `<name>_z`, the axial coordinate, equal to the cartesian $z$ coordinate, built from MFEM's
  [`mfem::CylindricalZCoefficient`](https://docs.mfem.org/html/namespacemfem.html#a3fd6a964e45636b5ef6071878ce39f56)

## Example Input File Syntax

!listing test/tests/mfem/functions/cylindrical_coefficients.i block=Functions FunctorMaterials

!syntax parameters /Functions/MFEMCoordinateTransformations

!syntax inputs /Functions/MFEMCoordinateTransformations

!syntax children /Functions/MFEMCoordinateTransformations

!if-end!

!else
!include mfem/mfem_warning.md