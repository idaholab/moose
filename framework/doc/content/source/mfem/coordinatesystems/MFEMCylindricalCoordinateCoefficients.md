# MFEMCylindrical

!if! function=hasCapability('mfem')

## Overview

Cylindrical coordinate scalar coefficients provider, for MFEM type cylindrical or 2D axisymmetric weak form formulations.

`MFEMCylindrical` derived from [MFEMCoordinateCoefficients](MFEMCoordinateCoefficients.md). It constructs and 
exposes the following built in scalar cylindrical coefficients

- `r` the radial component built from MFEM's [`mfem::CylindricalRadialCoefficient`](https://docs.mfem.org/html/coefficient_8hpp_source.html).
- `inv_r` the inverse radial coefficient 
- `two_pi_r` the full cylindrical measure factor {2\pi\text{r}}

The `two_pi_r` coefficient is constructed using [`mfem::TransformedCoefficient`](https://docs.mfem.org/html/classmfem_1_1TransformedCoefficient.html).

The `inv_r` coefficient is regularized as
!equation
{\text{inv\_r} = \sqrt\frac{1}{r^2 + \varepsilon^2}}

where $\varepsilon$ is specified by the `inv_r_eps` input parameter.


## Example Input File Syntax

!listing test/tests/mfem/coordinatesystems/cylindrical_coefficients.i block=CoordinateSystem

!syntax parameters /CoordinateSystem/MFEMCylindrical

!syntax inputs /CoordinateSystem/MFEMCylindrical

!syntax children /CoordinateSystem/MFEMCylindrical

!if-end!

!else
!include mfem/mfem_warning.md
