# MFEMCylindrical

!if! function=hasCapability('mfem')

## Overview

Cylindrical coordinate scalar coefficients provider, for MFEM type cylindrical or 2D axisymmetric weak form formulations.

`MFEMCylindrical` derived from [MFEMCoordinateCoefficients](MFEMCoordinateCoefficients.md). It constructs and 
exposes the following built in scalar cylindrical coefficients

- `r` the radial component built from MFEM's [`mfem::CylindricalRadialCoefficient`](https://docs.mfem.org/html/coefficient_8hpp_source.html).
- `inv_r` the inverse radial coefficient 
- `two_pi_r` the full cylindrical measure factor {2\pi\text{r}}
- `measure_weight` the reduced measure weight `r`, used when the costant factor {2\pi} is ommitted from the weak form 

The `two_pi_r` and `measure_weight` are constructed using [`mfem::TransformedCoefficient`](https://docs.mfem.org/html/classmfem_1_1TransformedCoefficient.html).

The `inv_r` coefficient is regularized as
!equation
{\text{inv\_r} = \frac{1}{r + \varepsilon}}

where $\varepsilon$ is specified by the `inv_r_eps` input parameter.


## Example Input File Syntax

!listing test/tests/mfem/coordinatesystems/cylindrical_coefficients.i block=Coordinatesystem

!syntax parameters /Coordinatesystems/MFEMCylindrical

!syntax inputs /Coordinatesystems/MFEMCylindrical

!syntax children /Coordinatesystems/MFEMCylindrical

!if-end!

!else
!include mfem/mfem_warning.md
