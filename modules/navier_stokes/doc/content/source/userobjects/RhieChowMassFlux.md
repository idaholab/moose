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

The pressure-gradient term used by [LinearFVMomentumPressure.md] kernels comes from the kernel's
gradient method. For [FVReconstructedPressureGradient.md], this object computes conservative
Rhie-Chow face fluxes and reconstructs a cell velocity from the face-normal velocities. The gradient
method then infers the pressure gradient from the SIMPLE momentum relation. The cell-velocity
reconstruction uses the least-squares face-flux projection described in
[!cite](aguerre2018oscillation),

!equation
\left(\sum_f |\vec{S}_f| \vec{n}_f \otimes \vec{n}_f\right) \vec{u}_C =
\sum_f (\vec{u}_f \cdot \vec{n}_f) \vec{S}_f,

followed by

!equation
\nabla p_C = \frac{-\vec{u}_C - (H/A)_C}{(1/A)_C}.

The reconstructed gradient is available after the first pressure correction; until then the gradient
method falls back to its base gradient method. The feedback is under-relaxed with
[!param](/UserObjects/RhieChowMassFlux/reconstructed_pressure_gradient_relaxation). Set
[!param](/UserObjects/RhieChowMassFlux/momentum_pressure_kernel) so Rhie-Chow uses the same pressure
gradient field as the momentum predictor while constructing H/A.

!syntax parameters /UserObjects/RhieChowMassFlux

!syntax inputs /UserObjects/RhieChowMassFlux

!syntax children /UserObjects/RhieChowMassFlux
