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

!syntax parameters /UserObjects/RhieChowMassFlux

!syntax inputs /UserObjects/RhieChowMassFlux

!syntax children /UserObjects/RhieChowMassFlux
