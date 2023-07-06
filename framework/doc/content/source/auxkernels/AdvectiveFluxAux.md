# AdvectiveFluxAux

!syntax description /AuxKernels/AdvectiveFluxAux

## Description

The `AdvectiveFluxAux` AuxKernel is used to compute a component of an advective flux vector. The flux is computed as

!equation
(\\vec{J} = \\vec{v} u \\cdot \\vec{n})

where $\\vec{J}$ is the advection flux vector, $u$ is the advected quantity, $\\vec{v}$ is the velocity, and $\\vec{n}$ is the normal for the desired component which can be the x, y, z axis or the normal direction (only near boundaries).

This auxkernel supports CONSTANT and FIRST MONOMIAL AuxVariable types. The advection velocity is required with [!param](/AuxKernels/AdvectiveFluxAux/vel_x) for advection flux calculation. [!param](/Postprocessors/SideAdvectiveFluxIntegral/vel_y) and [!param](/Postprocessors/SideAdvectiveFluxIntegral/vel_z) are needed for 2D and 3D simulations. The advected quantity can be either a variable[!param](/AuxKernels/AdvectiveFluxAux/advected_variable) or material property[!param](/AuxKernels/AdvectiveFluxAux/advected_mat_prop).

!alert warning
The expression of the advective flux in this object is generic, as described, and may differ from the advective flux implemented in your physics implementation. If so, you may not use this object to compute the advective flux.

## Example Input Syntax

!listing test/tests/auxkernels/advection_flux/advection_flux_fe.i block=AuxKernels

!syntax parameters /AuxKernels/AdvectiveFluxAux

!syntax inputs /AuxKernels/AdvectiveFluxAux

!syntax children /AuxKernels/AdvectiveFluxAux
