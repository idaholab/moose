# FVReconstructedPressureGradient

!syntax description /FVGradientMethods/FVReconstructedPressureGradient

## Overview

This gradient method is intended for linear finite-volume SIMPLE solves using
[RhieChowMassFlux.md]. It uses the configured base gradient method until Rhie-Chow has reconstructed
cell velocities from conservative face fluxes. It then combines those cell velocities with the latest
cell-centered H/A and 1/A data to infer the pressure gradient used by [LinearFVMomentumPressure.md]
and the H/A construction.

!syntax parameters /FVGradientMethods/FVReconstructedPressureGradient

!syntax inputs /FVGradientMethods/FVReconstructedPressureGradient

!syntax children /FVGradientMethods/FVReconstructedPressureGradient
