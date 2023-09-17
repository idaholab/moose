# PINSFVRhieChowInterpolatorSegregated

!syntax description /UserObjects/PINSFVRhieChowInterpolatorSegregated

## Overview

Similarly to [INSFVRhieChowInterpolatorSegregated.md], this object is responsible for generating the following fields for a [SIMPLE.md]-type segregated solver for the porous medium Navier-Stokes equations:

- $A^{-1}$ (inverse of the matrix diagonal) used as a diffusivity for the pressure equation.
  This field is stored in a cell-based functor, so centroid values are easy to access but
  face values need to be reconstructed.
- $A^{-1}H(u)$ whose divergence is used as a source in the pressure equation. This field is
  stored in a face-based functor, so face values are easy to access, but cell-center values
  need to be reconstructed.

This object operates on the matrices and right hand sides of the linearized momentum equations.
The main difference between this object and [INSFVRhieChowInterpolatorSegregated.md] is that
this object needs to account for the fact that in the porous medium equations there is a
porosity multiplier on the pressure gradient term in the momentum equation: $-\epsilon \nabla p$.

!syntax parameters /UserObjects/PINSFVRhieChowInterpolatorSegregated

!syntax inputs /UserObjects/PINSFVRhieChowInterpolatorSegregated

!syntax children /UserObjects/PINSFVRhieChowInterpolatorSegregated
