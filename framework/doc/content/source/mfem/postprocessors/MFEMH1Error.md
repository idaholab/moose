# MFEMH1Error

!if! function=hasCapability('mfem')

## Overview

Postprocessor for calculating the H1 error of a scalar $H^1$ conforming source variable compared to
a scalar function $u_{ex}$, given with its gradient:

!equation
\left\Vert u_{ex} - u_{h}\right\Vert_{\mathrm{H1}} = \left( \left\Vert u_{ex} - u_{h}\right\Vert_{\mathrm{L2}}^2 + \left\Vert \nabla u_{ex} - \nabla u_{h}\right\Vert_{\mathrm{L2}}^2 \right)^{1/2}

where $u_{h} \in H^1$. The gradient $\nabla u_{ex}$ is given by
[!param](/Postprocessors/MFEMH1Error/gradient), a vector function with one component per spatial
dimension of the mesh. The quadrature rule is set by
[!param](/Postprocessors/MFEMH1Error/quadrature_order), as described in
[MFEMNormPostprocessor.md].

## Example Input File Syntax

In this example, the H1 error of the solution `u` of a Poisson problem is computed against its exact
solution `u_exact`, whose gradient is `grad_u_exact`.

!listing test/tests/mfem/postprocessors/poisson_errors.i block=Postprocessors/h1_error

!syntax parameters /Postprocessors/MFEMH1Error

!syntax inputs /Postprocessors/MFEMH1Error

!syntax children /Postprocessors/MFEMH1Error

!if-end!

!else
!include mfem/mfem_warning.md
