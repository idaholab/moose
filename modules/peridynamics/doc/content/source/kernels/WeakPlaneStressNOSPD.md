# Kernel for Weak Plane Stress Model Based on the Form I of the Horizon-Stabilized Peridynamic Correspondence Formulation

## Description

The `WeakPlaneStressNOSPD` Kernel calculates the residual and Jacobian entries for the out-of-plane strain variable in the weak formulation of plane stress analysis. The in-plane displacement variables are governed by `HorizonStabilizedFormISmallStrainMechanicsNOSPD` for small strain analysis and `HorizonStabilizedFormIFiniteStrainMechanicsNOSPD` for finite strain analysis.

!syntax parameters /Kernels/WeakPlaneStressNOSPD

!syntax inputs /Kernels/WeakPlaneStressNOSPD

!syntax children /Kernels/WeakPlaneStressNOSPD
