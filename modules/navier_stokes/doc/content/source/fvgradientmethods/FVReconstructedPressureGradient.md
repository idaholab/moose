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
the next momentum predictor reads it. The method owns a persistent relaxed feedback
field that is updated exactly once per pressure corrector; repeated gradient
evaluations within an iteration are idempotent and do not change the stored field.

To use this method, configure [RhieChowMassFlux.md] with
[!param](/UserObjects/RhieChowMassFlux/momentum_pressure_kernel) so the momentum
pressure kernel and Rhie-Chow share the same pressure gradient field.

!syntax parameters /FVGradientMethods/FVReconstructedPressureGradient

!syntax inputs /FVGradientMethods/FVReconstructedPressureGradient

!syntax children /FVGradientMethods/FVReconstructedPressureGradient
