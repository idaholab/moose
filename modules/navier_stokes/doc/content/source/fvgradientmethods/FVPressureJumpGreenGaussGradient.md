# FVPressureJumpGreenGaussGradient

!syntax description /FVGradientMethods/FVPressureJumpGreenGaussGradient

## Description

`FVPressureJumpGreenGaussGradient` computes the cell pressure gradient without interpreting a
prescribed zero-thickness baffle pressure jump as a resolved pressure gradient. On an ordinary
internal face it uses the standard interpolated face pressure. On a baffle face it interpolates
the pressure separately on the two sides.

For an interface between cells $P$ and $N$, let $J_P=p_f^P-p_f^N$ be the pressure jump oriented
from $P$ to $N$. If $w_P$ and $w_N$ are the geometric interpolation weights, the two face
pressures are

\begin{equation}
\begin{aligned}
  p_f^P &= w_P p_P + w_N(p_N+J_P),\\
  p_f^N &= w_P(p_P-J_P) + w_Np_N.
\end{aligned}
\end{equation}

Their difference is $J_P$. When the jump is zero, both expressions reduce exactly to the ordinary
Green-Gauss face pressure. Each value contributes only to the surface sum of the cell on its side
of the baffle.

Use this method as the default [!param](/Variables/MooseLinearVariableFVReal/gradient_method) for
a pressure variable associated with a [PorousRhieChowMassFlux](PorousRhieChowMassFlux.md) that has
pressure-jump models. When [FVReconstructedPressureGradient](FVReconstructedPressureGradient.md)
is used for momentum-pressure coupling, also select this method with
[!param](/FVGradientMethods/FVReconstructedPressureGradient/base_gradient_method). Momentum
pressure kernels can then request the reconstructed method explicitly while diffusion corrections
and other pressure-gradient consumers obtain the jump-aware gradient from the variable.

## Example Input Syntax

This example defines the jump-aware solution gradient and the reconstructed coupling gradient.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=FVGradientMethods

The pressure variable selects the jump-aware method as its default gradient.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=Variables/pressure

The momentum pressure kernel requests the reconstructed method explicitly.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=LinearFVKernels/u_pressure

!syntax parameters /FVGradientMethods/FVPressureJumpGreenGaussGradient

!syntax inputs /FVGradientMethods/FVPressureJumpGreenGaussGradient

!syntax children /FVGradientMethods/FVPressureJumpGreenGaussGradient
