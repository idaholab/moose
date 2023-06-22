# AdvectiveFluxAux

!syntax description /AuxKernels/AdvectiveFluxAux

## Description

The `AdvectiveFluxAux` AuxKernel is used to compute the component of the flux vector for advection problems. The flux is computed as $(\\vec{J} = \\vec{v} u \\cdot \\vec{n})$, where $\\vec{J}$ is the advection flux vector, $u$ is the advected quantity, $\\vec{v}$ is the velocity, and $\\vec{n}$ is the desired component including x, y, z and normal direction. This kernel supports CONSTANT and FIRST MONOMIAL AuxVariable types. The advection velocity is required with [!param](/AuxKernels/AdvectiveFluxAux/vel_x) for advection flux calculation. [!param](/Postprocessors/SideAdvectiveFluxIntegral/vel_y) and [!param](/Postprocessors/SideAdvectiveFluxIntegral/vel_z) are needed for 2D and 3D simulations. The advected quantity can be either a variable[!param](/AuxKernels/AdvectiveFluxAux/advected_variable) or material property[!param](/AuxKernels/AdvectiveFluxAux/advected_mat_prop).

## Example Input Syntax

!listing test/tests/auxkernels/advection_flux/advection_flux_fe.i block=AuxKernels

!syntax parameters /AuxKernels/AdvectiveFluxAux

!syntax inputs /AuxKernels/AdvectiveFluxAux

!syntax children /AuxKernels/AdvectiveFluxAux
