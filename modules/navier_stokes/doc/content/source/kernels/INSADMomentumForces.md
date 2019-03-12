# INSADMomentumForces

This object adds a $-\rho\vec g -\vec f$ term to the
incompressible Navier Stokes momentum equation where $\rho$ is the density,
$\vec g$ is a gravity vector, and $\vec f$ represents an optionally user
specified body vector force. Note that the gravity and body forces are specified
through inputs to the [`INSADMaterial`](/INSADMaterial.md).

!syntax description /Kernels/INSADMomentumForces<RESIDUAL>

!syntax parameters /Kernels/INSADMomentumForces<RESIDUAL>

!syntax inputs /Kernels/INSADMomentumForces<RESIDUAL>

!syntax children /Kernels/INSADMomentumForces<RESIDUAL>
