# INSPressureGradientBC

## Overview

`INSPressureGradientBC` adds the exterior-face contribution from integrating the pressure
gradient in a momentum equation by parts. For momentum component $i$, integration by parts gives

!equation
\int_{\Omega} \psi \frac{\partial p}{\partial x_i}\,d\Omega =
-\int_{\Omega} p \frac{\partial \psi}{\partial x_i}\,d\Omega +
\int_{\partial\Omega} p n_i \psi\,d\Gamma,

where $p$ is the coupled pressure, $\psi$ is a test function for the momentum component, and
$n_i$ is component $i$ of the outward unit normal. This boundary condition implements the final
term on the selected external boundaries:

!equation
R_{\Gamma} = \int_{\Gamma} p n_i \psi\,d\Gamma.

Use this object with [PressureGradient.md] when
[!param](/Kernels/PressureGradient/integrate_p_by_parts) is `true`. If the momentum variable is
discontinuous, [INSPressureGradientDGKernel.md] supplies the corresponding contribution on
interior faces. `INSPressureGradientBC` does not prescribe a pressure value; it uses the current
value of the coupled [!param](/BCs/INSPressureGradientBC/pressure) variable. The
[!param](/BCs/INSPressureGradientBC/component) parameter selects the momentum direction: `0` for
$x$, `1` for $y$, and `2` for $z$.

## Example

The following block applies the pressure contribution to the $y$-momentum equation on the inlet
and wall boundaries:

!listing modules/navier_stokes/test/tests/finite_element/ins/dg/channel-flow/mass-energy-conservation-stokes.i block=BCs/implicit_pressure_y_in_and_walls

!syntax parameters /BCs/INSPressureGradientBC

!syntax inputs /BCs/INSPressureGradientBC

!syntax children /BCs/INSPressureGradientBC
