# Peridynamic Plane Stress Kernel

## Description

Kernel `WeakPlaneStressNOSPD` calculates the residual and Jacobian entries for the out-of-plane strain variable in weak formulation of plane stress analysis. The in-plane displacement variables is governed by the `SmallStrainMechanicsNOSPD` for small strain analysis and the `FiniteStrainMechanicsNOSPD` for finite strain analysis.

!syntax parameters /Kernels/WeakPlaneStressNOSPD

!syntax inputs /Kernels/WeakPlaneStressNOSPD

!syntax children /Kernels/WeakPlaneStressNOSPD
