# MFEML1Error

!if! function=hasCapability('mfem')

## Overview

Postprocessor for calculating the L1 error of a scalar $H^1$ or $L^2$ conforming source variable
compared to a scalar function.

!equation
\left\Vert u_{ex} - u_{h}\right\Vert_{\mathrm{L1}} = \int_\Omega \left| u_{ex} - u_{h} \right| \, \mathrm{d}V

where $u_{h} \in H^1 \lor L^2$ and $u_{ex}$ is a scalar function. The quadrature rule is set by
[!param](/Postprocessors/MFEML1Error/quadrature_order), as described in
[MFEMNormPostprocessor.md].

## Example Input File Syntax

In this example, the L1 error of the solution `u` of a Poisson problem is computed against its exact
solution `u_exact`.

!listing test/tests/mfem/postprocessors/poisson_errors.i block=Postprocessors/l1_error

!syntax parameters /Postprocessors/MFEML1Error

!syntax inputs /Postprocessors/MFEML1Error

!syntax children /Postprocessors/MFEML1Error

!if-end!

!else
!include mfem/mfem_warning.md
