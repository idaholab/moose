# FVReconstructedPressureGradient

## Overview

This gradient method is intended for linear finite-volume SIMPLE solves using
[RhieChowMassFlux.md]. It uses the configured base gradient method until Rhie-Chow has reconstructed
cell velocities from conservative face fluxes. It then combines those cell velocities with the latest
cell-centered H/A and 1/A data to infer the pressure gradient used by [LinearFVMomentumPressure.md]
and the H/A construction.

The reconstruction follows the flux-reconstruction idea of [!cite](aguerre2018oscillation):
the Rhie-Chow face fluxes define face-normal velocities, a least-squares projection
recovers a cell velocity, and the cell pressure gradient is inferred from
$\vec{u}_C = -(H/A)_C - (1/A)_C \nabla p_C$.

!syntax parameters /FVGradientMethods/FVReconstructedPressureGradient

!syntax inputs /FVGradientMethods/FVReconstructedPressureGradient

!syntax children /FVGradientMethods/FVReconstructedPressureGradient
