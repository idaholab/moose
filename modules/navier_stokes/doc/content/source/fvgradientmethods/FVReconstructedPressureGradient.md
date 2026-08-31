# FVReconstructedPressureGradient

## Overview

FVReconstructedPressureGradient is a gradient method for linear finite-volume SIMPLE
solves that works with [RhieChowMassFlux.md]. After Rhie-Chow has assembled conservative
face fluxes, it reconstructs the cell pressure gradient by removing the contribution
from the previous velocity gradient, reconstructing a cell velocity, and recovering the
pressure gradient from the momentum balance.

Before the first pressure correction, the registered pressure gradient field uses the
configured base gradient method. After that, the reconstructed gradient replaces the
base gradient for the momentum predictor and the H/A construction.

The reconstructed gradient is relaxed between SIMPLE iterations using
[!param](/FVGradientMethods/FVReconstructedPressureGradient/gradient_relaxation) before
the next momentum predictor reads it.

To use this method, configure [RhieChowMassFlux.md] with
[!param](/UserObjects/RhieChowMassFlux/momentum_pressure_kernel) so the momentum
pressure kernel and Rhie-Chow share the same pressure gradient field. When the
pressure diffusion kernel uses nonorthogonal correction, also set
[!param](/UserObjects/RhieChowMassFlux/pressure_projection_method) to `consistent`.

On strongly nonorthogonal meshes, the reconstruction is only as accurate as the
decomposition of the conservative face fluxes into pressure-gradient and velocity
components. In those cases, a standard gradient method such as
[FVGreenGaussGradient.md] may be a better choice.

!syntax parameters /FVGradientMethods/FVReconstructedPressureGradient

!syntax inputs /FVGradientMethods/FVReconstructedPressureGradient

!syntax children /FVGradientMethods/FVReconstructedPressureGradient
