# SCMRZPinQPrimeAux

!syntax description /AuxKernels/SCMRZPinQPrimeAux

## Description

!! Intentional comment to provide extra spacing

The `SCMRZPinQPrimeAux` AuxKernel is used to compute the linear heat rate (W/m) on the surface of a fuel pin that is modeled with a 2D-RZ axisymmetric mesh.
It's a kernel that inherits from `DiffusionFluxAux` to calculate the flux, but in addition it multiplies with the pin diameter. The user needs to provide the
diffusion coefficient and the temperature variable.

!syntax parameters /AuxKernels/SCMRZPinQPrimeAux

!syntax inputs /AuxKernels/SCMRZPinQPrimeAux

!syntax children /AuxKernels/SCMRZPinQPrimeAux
