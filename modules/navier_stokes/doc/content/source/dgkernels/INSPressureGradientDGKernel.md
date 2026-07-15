# INSPressureGradientDGKernel

## Overview

`INSPressureGradientDGKernel` adds the interior-face contribution from integrating the pressure
gradient in a momentum equation by parts when using a discontinuous finite element space. For
momentum component $i$, the element-volume and face terms are

!equation
\int_{\Omega} \psi \frac{\partial p}{\partial x_i}\,d\Omega =
-\int_{\Omega} p \frac{\partial \psi}{\partial x_i}\,d\Omega +
\int_{\partial\Omega} p n_i \psi\,d\Gamma.

On an interior face $F$ shared by an element and its neighbor, this object uses the centered
pressure

!equation
\{p\} = \frac{1}{2}\left(p^{\mathrm{elem}} + p^{\mathrm{neighbor}}\right).

With the face normal $\boldsymbol{n}$ pointing from the element to the neighbor, the residual
contributions are

!equation
R_F^{\mathrm{elem}} = \int_F \{p\} n_i \psi^{\mathrm{elem}}\,d\Gamma

and

!equation
R_F^{\mathrm{neighbor}} = -\int_F \{p\} n_i \psi^{\mathrm{neighbor}}\,d\Gamma.

[PressureGradient.md] with [!param](/Kernels/PressureGradient/integrate_p_by_parts) set to `true`
supplies the element-volume term. [INSPressureGradientBC.md] supplies the corresponding pressure
contribution on external boundaries. The
[!param](/DGKernels/INSPressureGradientDGKernel/component) parameter selects the momentum
direction: `0` for $x$, `1` for $y$, and `2` for $z$.

## Example

The following blocks add the volume and interior-face pressure terms to the $x$-momentum
equation:

!listing modules/navier_stokes/test/tests/finite_element/ins/compatibility/dg-stokes.i block=Kernels/momentum_x_pressure DGKernels/momentum_x_pressure

!syntax parameters /DGKernels/INSPressureGradientDGKernel

!syntax inputs /DGKernels/INSPressureGradientDGKernel

!syntax children /DGKernels/INSPressureGradientDGKernel
