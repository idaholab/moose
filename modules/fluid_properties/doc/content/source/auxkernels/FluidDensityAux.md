# FluidDensityAux

!syntax description /AuxKernels/FluidDensityAux

# Description

Calculate the fluid density from the fluid properties object as a function of pressure
and temperature.

!syntax parameters /AuxKernels/FluidDensityAux

!syntax inputs /AuxKernels/FluidDensityAux

!syntax children /AuxKernels/FluidDensityAux

## Example

Below is an example for computing the density using the `eos` fluid property object
based on nonlinear variables that represent pressure and temperature that are named
`pressure` and `temperature`, respectively.

!listing test/tests/auxkernels/fluid_density_aux.i
  block=AuxKernels/density
