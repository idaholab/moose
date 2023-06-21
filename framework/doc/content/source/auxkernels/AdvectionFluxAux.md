# DiffusionFluxAux

!syntax description /AuxKernels/AdvectiveFluxAux

## Description

The `AdvectiveFluxAux` AuxKernel is used to compute the components of the flux vector for advection problems. The flux is computed as $(\\vec{J} = \\vec{v} u \\cdot \\vec{n})$, where $\\vec{J}$ is the advection flux vector, $u$ is the advected variable, $\\vec{v}$ is the advected velocity, and $\\vec{n}$ is the desired component.


!syntax parameters /AuxKernels/AdvectiveFluxAux

!syntax inputs /AuxKernels/AdvectiveFluxAux

!syntax children /AuxKernels/AdvectiveFluxAux
