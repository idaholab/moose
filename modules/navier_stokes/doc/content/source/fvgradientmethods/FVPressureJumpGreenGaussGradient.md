# FVPressureJumpGreenGaussGradient

!syntax description /FVGradientMethods/FVPressureJumpGreenGaussGradient

## Description

`FVPressureJumpGreenGaussGradient` computes the cell pressure gradient without interpreting a
prescribed zero-thickness baffle pressure jump as a resolved pressure gradient. On an ordinary
internal face it uses the standard interpolated face pressure. On a baffle face it recovers the
two one-sided face pressures from the same conservative half-cell flux relation used by
[LinearFVPressureCorrectionDiffusionJump](LinearFVPressureCorrectionDiffusionJump.md).

For an interface between cells $P$ and $N$, let $J_P=p_f^P-p_f^N$ be the pressure jump oriented
from $P$ to $N$. With the half-cell conductances $G_P$ and $G_N$, explicit corrections $r_P$ and
$r_N$, one-sided $\boldsymbol H/\boldsymbol A$ mass fluxes $h_P$ and $h_N$, and common oriented
face mass flux $Q_f$, the two face pressures are

\begin{equation}
\begin{aligned}
  p_f^P &= p_P-\frac{Q_f+h_P+r_P}{G_P},\\
  p_f^N &= p_N+\frac{Q_f+h_N+r_N}{G_N}.
\end{aligned}
\end{equation}

The flux $Q_f$ and the series-weighted $\boldsymbol H/\boldsymbol A$ flux are derived in
[LinearFVPressureCorrectionDiffusionJump](LinearFVPressureCorrectionDiffusionJump.md). The terms
$h_s$ make each face pressure follow the body-force pressure slope of its own side, so a fluid at
rest across a porosity or density jump has the hydrostatic face pressures.

The explicit corrections use the previously published geometric pressure gradient, matching the
deferred nonorthogonal correction in the pressure equation. The recovered one-sided face values
satisfy both the prescribed jump $p_f^P-p_f^N=J_P$ and equal flux from the two half cells. Each
value contributes only to the Green-Gauss surface sum of the cell on its side of the baffle. If
the half-cell geometry is degenerate, the method falls back to jump-adjusted geometric
interpolation.

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
