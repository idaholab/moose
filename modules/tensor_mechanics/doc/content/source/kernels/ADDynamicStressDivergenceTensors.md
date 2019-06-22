# ADDynamicStressDivergenceTensors

!syntax description /Kernels/ADDynamicStressDivergenceTensors

## Description

This class computes the stress divergence and the stiffness proportional Rayleigh damping for  
structural dynamics problems. Each ADDynamicStressDivergenceTensors input block computes force in  
one direction. So, a separate ADDynamicStressDivergenceTensors input block should be set up for  
each coordinate direction. The [DynamicTensorMechanics](/DynamicTensorMechanicsAction.md) action  
automatically sets up the ADDynamicStressDivergenceTensors input block in all coordinate directions.  
More information about the residual calculation and usage can be found at [Dynamics](Dynamics.md).

!syntax parameters /Kernels/ADDynamicStressDivergenceTensors

!syntax inputs /Kernels/ADDynamicStressDivergenceTensors

!syntax children /Kernels/ADDynamicStressDivergenceTensors
