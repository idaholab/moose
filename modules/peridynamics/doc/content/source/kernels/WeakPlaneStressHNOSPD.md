# Peridynamic Plane Stress Kernel

## Description

The `WeakPlaneStressHNOSPD` Kernel calculates the residual and Jacobian entries for the out-of-plane strain variable in weak formulation of plane stress analysis. The in-plane displacement variables are governed by `HorizonStabilizedSmallStrainMechanicsNOSPD` for small strain analysis and `HorizonStabilizedFiniteStrainMechanicsNOSPD` for finite strain analysis.

!syntax parameters /Kernels/WeakPlaneStressHNOSPD

!syntax inputs /Kernels/WeakPlaneStressHNOSPD

!syntax children /Kernels/WeakPlaneStressHNOSPD
