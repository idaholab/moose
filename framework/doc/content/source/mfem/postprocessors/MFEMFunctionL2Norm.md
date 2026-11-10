# MFEMFunctionL2Norm

!if! function=hasCapability('mfem')

## Overview

Postprocessor for calculating the L2 norm of a scalar function $f$ over the MFEM mesh:

!equation
\left\Vert f\right\Vert_{\mathrm{L2}} = \left( \int_\Omega f^2 \, \mathrm{d}V \right)^{1/2}

The norm of an exact solution turns an error computed by [MFEML2Error.md] into a relative error.
Without a variable, there is no element order to choose a quadrature rule from, so the order of the
rule used on every element must be given by
[!param](/Postprocessors/MFEMFunctionL2Norm/quadrature_order).

## Example Input File Syntax

In this example, the L2 norm of the exact solution `u_exact` of a Poisson problem is computed with
the quadrature rule of order 5.

!listing test/tests/mfem/postprocessors/poisson_errors.i block=Postprocessors/u_exact_norm

!syntax parameters /Postprocessors/MFEMFunctionL2Norm

!syntax inputs /Postprocessors/MFEMFunctionL2Norm

!syntax children /Postprocessors/MFEMFunctionL2Norm

!if-end!

!else
!include mfem/mfem_warning.md
