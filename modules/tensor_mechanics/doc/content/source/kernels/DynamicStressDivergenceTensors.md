# DynamicStressDivergenceTensors

!syntax description /Kernels/DynamicStressDivergenceTensors

## Description

This class computes the stress divergence and the stiffness proportional Rayleigh damping for structual dynamics problems. Each DynamicStressDivergenceTensors input block computes force in one direction. So, a separate DynamicStressDivergenceTensors input block should be set up for each coordinate direction. The [DynamicTensorMechanics](/DynamicTensorMechanicsAction.md) action automatically sets up the DynamicStressDivergenceTensors input block in all coordinate direction. More information about the residual calculation and usage can be found at [Dynamics](Dynamics.md).

!syntax parameters /Kernels/DynamicStressDivergenceTensors

!syntax inputs /Kernels/DynamicStressDivergenceTensors

!syntax children /Kernels/DynamicStressDivergenceTensors
