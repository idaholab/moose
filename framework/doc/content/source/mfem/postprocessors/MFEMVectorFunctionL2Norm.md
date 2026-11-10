# MFEMVectorFunctionL2Norm

!if! function=hasCapability('mfem')

## Overview

Postprocessor for calculating the L2 norm of a vector function $\vec{f}$ over the MFEM mesh:

!equation
\left\Vert \vec{f}\right\Vert_{\mathrm{L2}} = \left( \int_\Omega \vec{f} \cdot \vec{f} \, \mathrm{d}V \right)^{1/2}

The norm of an exact solution turns an error computed by [MFEMVectorL2Error.md] into a relative
error. Without a variable, there is no element order to choose a quadrature rule from, so the order
of the rule used on every element must be given by
[!param](/Postprocessors/MFEMVectorFunctionL2Norm/quadrature_order).

## Example Input File Syntax

In this example, the L2 norm of the gradient `grad_u_exact` of the exact solution of a Poisson
problem is computed with the quadrature rule of order 5.

!listing test/tests/mfem/postprocessors/poisson_errors.i block=Postprocessors/grad_u_exact_norm

!syntax parameters /Postprocessors/MFEMVectorFunctionL2Norm

!syntax inputs /Postprocessors/MFEMVectorFunctionL2Norm

!syntax children /Postprocessors/MFEMVectorFunctionL2Norm

!if-end!

!else
!include mfem/mfem_warning.md
