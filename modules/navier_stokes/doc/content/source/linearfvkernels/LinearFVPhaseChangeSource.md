# LinearFVPhaseChangeSource

!syntax description /LinearFVKernels/LinearFVPhaseChangeSource

## Overview

This elemental kernel adds the apparent heat-capacity latent-heat contribution to a
temperature-based energy equation solved with the linear finite volume discretization:

\begin{equation}
\rho L \frac{\partial f}{\partial T} \frac{\partial T}{\partial t},
\end{equation}

where $\rho$ is the mixture density, $L$ the latent heat of fusion, and $f$ the liquid
fraction defined over the mushy interval $[T_{sol}, T_{liq}]$ using the normalized
temperature

\begin{equation}
s = \min\left(\max\left(\frac{T - T_{sol}}{T_{liq} - T_{sol}}, 0\right), 1\right).
\end{equation}

Two liquid-fraction shapes are available through the
[!param](/LinearFVKernels/LinearFVPhaseChangeSource/smoothing) parameter:

- `smooth` (default): a smoothstep $f(s) = 3s^2 - 2s^3$, giving
  $\partial f / \partial T = 6 s (1 - s) / (T_{liq} - T_{sol})$, which vanishes
  continuously at both ends of the mushy interval;
- `sharp`: a linear liquid fraction $f(s) = s$, giving a constant
  $\partial f / \partial T = 1 / (T_{liq} - T_{sol})$ inside the interval and zero
  outside.

Both variants release the same total latent heat across the interval. The matrix
contribution treats the temperature time derivative implicitly using the time
integrator of the system, while $\partial f / \partial T$ is evaluated using the
current temperature iterate.

!alert note
This kernel is intended for temperature-based solves only. It must not be combined
with a total-enthalpy formulation, in which the latent heat is already contained in
the solved variable.

## Example input syntax

!listing test/tests/finite_volume/ins/solidification/1d-stefan_linearfv.i block=LinearFVKernels

!syntax parameters /LinearFVKernels/LinearFVPhaseChangeSource

!syntax inputs /LinearFVKernels/LinearFVPhaseChangeSource

!syntax children /LinearFVKernels/LinearFVPhaseChangeSource
