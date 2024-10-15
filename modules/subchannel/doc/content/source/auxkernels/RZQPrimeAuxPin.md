# RZQPrimeAuxPin

!syntax description /AuxKernels/RZQPrimeAuxPin

## Description

<!-- -->

The `RZQPrimeAuxPin` AuxKernel is used to compute the linear heat rate (w/m) on the surface of a fuel pin that is modeled with a 2D-RZ axi-symmetric mesh.
It's a kernel that inherits from `DiffusionFluxAux` to calculate the flux, but in addition it multiplies with the pin diameter. The user needs to provide the
diffusion coefficient and the temperature variable.

## Example Input File Syntax

!listing /examples/coupling/1pinSquare_thermomech_SC/one_pin_problem_sub.i block=AuxKernels language=cpp

!syntax parameters /AuxKernels/RZQPrimeAuxPin

!syntax inputs /AuxKernels/RZQPrimeAuxPin

!syntax children /AuxKernels/RZQPrimeAuxPin
