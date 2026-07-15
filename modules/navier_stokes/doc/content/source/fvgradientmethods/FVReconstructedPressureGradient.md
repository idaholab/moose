# FVReconstructedPressureGradient

## Overview

This gradient method is intended for linear finite-volume SIMPLE solves using
[RhieChowMassFlux.md]. It uses the configured base gradient method until Rhie-Chow has reconstructed
pressure gradients from conservative face fluxes. The reconstructed gradient is relaxed as feedback
between SIMPLE iterations before it is used by [LinearFVMomentumPressure.md] and the H/A
construction.

Use pressure-field gradient methods directly for pressure-gradient feedback. Conservative
Rhie-Chow face fluxes are suitable for velocity and flux reconstruction, but should not be treated
as pointwise pressure-gradient data on nonorthogonal meshes.

Rhie-Chow removes the previous velocity-gradient contribution from the face flux, reconstructs a
cell velocity, and recovers the pressure gradient from the momentum balance. When the pressure
diffusion kernel uses nonorthogonal correction,
[!param](/UserObjects/RhieChowMassFlux/pressure_projection_method) must be set to `consistent`.

!syntax parameters /FVGradientMethods/FVReconstructedPressureGradient

!syntax inputs /FVGradientMethods/FVReconstructedPressureGradient

!syntax children /FVGradientMethods/FVReconstructedPressureGradient
