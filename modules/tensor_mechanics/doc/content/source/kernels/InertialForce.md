# InertialForce

!syntax description /Kernels/InertialForce

## Description

This class computes the inertial force using a consistent mass matrix and also computes the mass proportional Rayleigh damping. More information about the residual calculation and usage can be found at [Dynamics](Dynamics.md). Each InertialForce kernel calculates the force only along one coordinate direction. So, a separate InertialForce input block should be set up for each coordinate direction.

!syntax parameters /Kernels/InertialForce

!syntax inputs /Kernels/InertialForce

!syntax children /Kernels/InertialForce
