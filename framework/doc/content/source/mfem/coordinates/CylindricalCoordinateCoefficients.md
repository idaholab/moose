# Cylindrical

!if! function=hasCapability('mfem')

## Overview

Cylindrical coordinate MFEM coefficients provider, used to formulate reduced 2D axisymmetric weak forms.

`Cylindrical` derived from [MFEMCoordinateCoefficients](MFEMCoordinateCoefficients.md). It constructs and 
exposes the following built in scalar cylindrical coefficients

- `r` the radial component built from MFEM's [`mfem::CylindricalRadialCoefficient`](https://docs.mfem.org/html/coefficient_8hpp_source.html).
- `inv_r` the inverse radial coefficient 
- `two_pi_r`
- `measure_weight`

The `two_pi_r` and `measure_weight` are constructed using [`mfem::TransformedCoefficient`](https://docs.mfem.org/html/classmfem_1_1TransformedCoefficient.html).

The `inv_r` coefficient is regularized as
!equation
{\text{inv\_r} = \frac{1}{r + \varepsilon}}

where $\varepsilon$ is specified by the `inv_r_eps` input parameter.


## Example Input File Syntax

!listing graddiv.i block=Coordinates

!syntax parameters /Coordinates/Cylindrical

!syntax inputs /Coordinates/Cylindrical

!syntax children /Coordinates/Cylindrical

!if-end!

!else
!include mfem/mfem_warning.md
