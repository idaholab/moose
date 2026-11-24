# FVVelocityJumpGreenGaussGradient

!syntax description /FVGradientMethods/FVVelocityJumpGreenGaussGradient

## Description

`FVVelocityJumpGreenGaussGradient` computes the cell gradient of a superficial velocity component
without interpreting the velocity jump at a pressure baffle as a resolved slope. Mass conservation
keeps $\rho\mathbf{U}\cdot\mathbf{n}_f$ continuous across a baffle and density varies
continuously, but the tangential superficial velocity jumps wherever porosity jumps. An ordinary Green-Gauss gradient interpolates the two
cell values to the face and turns a jump $\Delta U$ into a slope of order $\Delta U/h$.

On an ordinary internal face this method uses the standard interpolated face value. On a baffle
face of the linked [PorousRhieChowMassFlux](PorousRhieChowMassFlux.md), each cell instead uses the
face value extrapolated from its own center,

\begin{equation}
  U_{i,f}^{s} = U_{i,s} + \nabla U_{i,s}\mathbin{\cdot}\left(\mathbf{x}_f-\mathbf{x}_s\right),
  \qquad s\in\{P,N\},
\end{equation}

which is exact for a field that is linear on each side. The gradient on the right is the
previously published one, so the method is a fixed-point iteration that converges over the
segregated iterations. Before the Rhie-Chow object is linked, the cell value is used.

[PorousRhieChowMassFlux](PorousRhieChowMassFlux.md) requires this method as the default
[!param](/Variables/MooseLinearVariableFVReal/gradient_method) of every velocity variable when it
has pressure-jump models, and links itself to the method. All consumers of the default velocity
gradient, including the nonorthogonal and deviatoric stress corrections and the lagged gradient in
[FVReconstructedPressureGradient](FVReconstructedPressureGradient.md), therefore see one-sided
gradients.

## Example Input Syntax

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=FVGradientMethods

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=Variables/superficial_u

!syntax parameters /FVGradientMethods/FVVelocityJumpGreenGaussGradient

!syntax inputs /FVGradientMethods/FVVelocityJumpGreenGaussGradient

!syntax children /FVGradientMethods/FVVelocityJumpGreenGaussGradient
