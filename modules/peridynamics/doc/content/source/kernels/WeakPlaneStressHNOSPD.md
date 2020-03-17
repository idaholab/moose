# Peridynamic Plane Stress Kernel

## Description

Kernel `WeakPlaneStressHNOSPD` calculates the residual and Jacobian entries for the out-of-plane strain variable in weak formulation of plane stress analysis. The in-plane displacement variables is governed by the `HorizonStabilizedSmallStrainMechanicsNOSPD` for small strain analysis and the `HorizonStabilizedFiniteStrainMechanicsNOSPD` for finite strain analysis.

!syntax parameters /Kernels/WeakPlaneStressHNOSPD

!syntax inputs /Kernels/WeakPlaneStressHNOSPD

!syntax children /Kernels/WeakPlaneStressHNOSPD
