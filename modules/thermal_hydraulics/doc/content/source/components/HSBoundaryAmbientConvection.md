# HSBoundaryAmbientConvection

This component is a
[heat structure boundary](thermal_hydraulics/component_groups/heat_structure_boundary.md)
that applies convective heat transfer boundary conditions.

## Usage

!template load file=heat_structure_boundary_usage.md.template name=HSBoundaryAmbientConvection

The parameter [!param](/Components/HSBoundaryAmbientConvection/T_ambient) gives the ambient temperature $T_\infty$, and
[!param](/Components/HSBoundaryAmbientConvection/htc_ambient) gives the heat transfer coefficient $\mathcal{H}$.

The parameter [!param](/Components/HSBoundaryAmbientConvection/scale_pp) specifies
the name of a post-processor $f$ that can scale the boundary conditions.

!syntax parameters /Components/HSBoundaryAmbientConvection

If this component is used with a cylindrical heat structure, the post-processor
*name*`_integral` is added, which gives the heat rate found by integrating this
heat flux over the boundary.

## Formulation

!include heat_structure_formulation.md

!include heat_structure_boundary_formulation_neumann.md

For convection boundary conditions, the incoming boundary heat flux $q_b$ is computed as

!equation
q_b = f \mathcal{H} (T_\infty - T) \eqc

where

- $\mathcal{H}$ is the heat transfer coefficient,
- $T$ is the temperature of the surface,
- $T_\infty$ is the ambient temperature, and
- $f$ is an optional scaling factor.

!syntax inputs /Components/HSBoundaryAmbientConvection

!syntax children /Components/HSBoundaryAmbientConvection
