# DiffusionFluxAux

!syntax description /AuxKernels/DiffusionFluxAux

## Description

The `DiffusionFluxAux` AuxKernel is used to compute the components of the flux vector for diffusion problems. The flux is computed as $J=-D\frac{\partial C}{\partial X}$, where $J$ is the diffusion flux vector, $D$ is the diffusivity or diffusion coefficient, $C$ is the concentration variable, and $X$ is the coordinate.

It supports the definition of the diffusivity with and without automatic differentiation (AD).

!alert warning
The expression of the diffusive flux in this object is generic, as described, and may differ from the diffusive flux in your specific physics implementation. If so, you may not use this object to compute the diffusive flux.

!syntax parameters /AuxKernels/DiffusionFluxAux

!syntax inputs /AuxKernels/DiffusionFluxAux

!syntax children /AuxKernels/DiffusionFluxAux
