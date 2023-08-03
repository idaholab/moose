# SideAdvectiveFluxIntegral

!syntax description /Postprocessors/SideAdvectiveFluxIntegral

## Description

The `SideAdvectiveFluxIntegral` postprocessor is used to compute the side integral of the advection flux component including x, y, z and normal. This kernel supports both finite element and finite volume variables.

For finite element method, the variable needs to be a continuous finite element type(e.g.LAGRANGE, HIERARCHIC, BERNSTEIN, SZABAB, RATIONAL_BERNSTEIN, CLOUGH, HERMITE, etc.), so that the side value can be estimated in MOOSE. The advection variable can be either a variable[!param](/Postprocessors/SideAdvectiveFluxIntegral/advected_variable) or material property[!param](/Postprocessors/SideAdvectiveFluxIntegral/advected_mat_prop).

For finite volume method, an advected quantity needs to be provided[!param](/Postprocessors/SideAdvectiveFluxIntegral/advected_quantity). The advection velocity is required with [!param](/Postprocessors/SideAdvectiveFluxIntegral/vel_x) for advection flux calculation. [!param](/Postprocessors/SideAdvectiveFluxIntegral/vel_y) and [!param](/Postprocessors/SideAdvectiveFluxIntegral/vel_z) are needed for 2D and 3D simulations.

!alert warning
The expression of the advective flux in this object is generic, as described, and may differ from the advective flux implemented in your physics implementation. If so, you may not use this object to compute the advective flux.

## Example Input Syntax

!listing test/tests/postprocessors/side_advection_flux_integral/side_advection_flux_integral.i block=Postprocessors

!syntax parameters /Postprocessors/SideAdvectiveFluxIntegral

!syntax inputs /Postprocessors/SideAdvectiveFluxIntegral

!syntax children /Postprocessors/SideAdvectiveFluxIntegral
