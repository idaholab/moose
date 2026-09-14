# RhieChowMassFlux

!syntax description /UserObjects/RhieChowMassFlux

## Overview

This object is responsible for generating the following fields for a [SIMPLE.md]-type
segregated solver:

- $A^{-1}$ (inverse of the matrix diagonal) which is used as a diffusivity for the pressure equation.
  This field is stored in a face-based functor, so face values are easy to access but
  cell values need to be reconstructed. This is mainly used in the pressure Poisson equation
  where only face values are queried.
- $A^{-1}H(u)$ whose divergence is used as a source in the pressure Poisson equation.
  This field is also stored in a face-based functor, so face values are easy to access,
  but cell-center values need to be reconstructed.
- $(\rho \vec{u} \vec{n})_{RC}$ which is the Rhie-Chow corrected face mass flux. This is
  also stored in a face-based functor, so face values are easy to access,
  but cell-center values need to be reconstructed.

Besides these capabilities, this user object is also responsible for reconstructing
cell velocities at the end of the pressure corrector step.
For more information on these fields and processes, we suggest visiting [SIMPLE.md].

The object enables the computation of the standard (SIMPLE) or consistent (SIMPLEC)
momentum projection matrix ($A^{-1}$) and neighbour face flux ($H(u)$) vector via
[!param](/UserObjects/RhieChowMassFlux/pressure_projection_method).
In general, SIMPLEC will be stable with higher relaxation factors for pressure than SIMPLE.
This is particularly useful in problems with slow-converging pressure fields,
such as those with high Reynolds numbers, complex geometries, viscous flows in narrow channels,
multiphase flows, problems with rapidly varying thermophysical properties, and,
in general, when using high-resolution grids.

### Reconstructed Pressure Gradient

The `reconstructed` pressure-gradient option uses the Aguerre reconstruction
([!cite](aguerre2018oscillation)) implemented by [FVReconstructedPressureGradient.md]. It is useful
when the conservative Rhie-Chow face flux is smooth, but the cell-centered velocity still exhibits
oscillations because its pressure gradient is not fully consistent with that face flux.

After a pressure correction, the method reconstructs a cell velocity that is compatible with the
corrected conservative face flux and then determines the pressure gradient required by the cell
momentum balance. The newly reconstructed gradient corrects the velocity immediately so that the
cell velocity and continuity-preserving face flux remain consistent. A relaxed version of that
gradient is used by the next momentum predictor to avoid introducing an abrupt change into the
pressure-velocity coupling. The accepted gradient is carried between time steps and is preserved
through time-step retries and restarts.

The following options control the reconstruction:

- [!param](/FVGradientMethods/FVReconstructedPressureGradient/gradient_relaxation) controls how
  strongly the newly reconstructed gradient changes the gradient used by the next momentum solve.
  The default value of `0.1` is deliberately conservative. Smaller values provide more damping and
  can improve robustness, but usually require more outer iterations. Values closer to `1` respond
  more quickly to the latest pressure correction, but can strengthen pressure-velocity oscillations.
- [!param](/FVGradientMethods/FVReconstructedPressureGradient/base_gradient_method) selects the
  ordinary pressure-gradient method used before the first reconstructed gradient is available. The
  default is `green-gauss`.

For example, the following tested input selects the reconstructed method through an input-file
variable and uses that selection for the pressure variable:

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/reconstructed-force-channel.i line=pressure_gradient_method

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/reconstructed-force-channel.i block=Variables/pressure FVGradientMethods/reconstructed

Use the same reconstructed pressure-gradient definition for every [LinearFVMomentumPressure.md]
component coupled to this pressure equation. Mixing gradient definitions among velocity components
would make the reconstructed cell velocity inconsistent with the coupled momentum balance. Other
equations and diagnostic quantities should continue to use an ordinary gradient method.

### Pressure Diffusion Interpolation

The [!param](/UserObjects/RhieChowMassFlux/pressure_diffusion_interpolation) parameter selects
whether `average` or `harmonic` interpolation is used when computing the face values of `Ainv`,
the $A^{-1}$ diffusion tensor in the pressure correction diffusion term. When using
[WCNSLinearFVFlowPhysics.md], select this through the `pressure_diffusion_interpolation` parameter
in the Physics block.

!syntax parameters /UserObjects/RhieChowMassFlux

!syntax inputs /UserObjects/RhieChowMassFlux

!syntax children /UserObjects/RhieChowMassFlux
