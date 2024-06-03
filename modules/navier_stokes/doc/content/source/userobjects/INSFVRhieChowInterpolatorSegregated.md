# INSFVRhieChowInterpolatorSegregated

!syntax description /UserObjects/INSFVRhieChowInterpolatorSegregated

## Overview

This object is responsible for generating the following fields for a [SIMPLENonlinearAssembly.md]-type
segregated solver:

- $A^{-1}$ (inverse of the matrix diagonal) which is used as a diffusivity for the pressure equation.
  This field is stored in a cell-based functor, so centroid values are easy to access but
  face values need to be reconstructed.
- $A^{-1}H(u)$ whose divergence is used as a source in the pressure equation. This field is
  stored in a face-based functor, so face values are easy to access, but cell-center values
  need to be reconstructed.

This object operates on the matrices and right hand sides of the linearized momentum equations.

!syntax parameters /UserObjects/INSFVRhieChowInterpolatorSegregated

!syntax inputs /UserObjects/INSFVRhieChowInterpolatorSegregated

!syntax children /UserObjects/INSFVRhieChowInterpolatorSegregated
