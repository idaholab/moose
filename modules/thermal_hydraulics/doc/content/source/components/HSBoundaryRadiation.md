# HSBoundaryRadiation

This component is a
[heat structure boundary](thermal_hydraulics/component_groups/heat_structure_boundary.md)
that applies radiative heat transfer boundary conditions.

## Usage

!template load file=heat_structure_boundary_usage.md.template name=HSBoundaryRadiation

The parameter [!param](/Components/HSBoundaryRadiation/T_ambient) gives the ambient temperature $T_\infty$,
[!param](/Components/HSBoundaryRadiation/emissivity) gives the surface emissivity $\epsilon$, and
[!param](/Components/HSBoundaryRadiation/view_factor) gives the view factor $F$.

The parameter [!param](/Components/HSBoundaryRadiation/scale_pp) specifies
the name of a post-processor $f$ that can scale the boundary conditions.

!syntax parameters /Components/HSBoundaryRadiation

If this component is used with a cylindrical heat structure, the post-processor
*name*`_integral` is added, which gives the heat rate found by integrating this
heat flux over the boundary.

## Formulation

!include heat_structure_formulation.md

!include heat_structure_boundary_formulation_neumann.md

For radiation boundary conditions, the incoming boundary heat flux $q_b$ is computed as

\begin{equation}
  q_b = f \sigma \epsilon F (T^4_\infty - T^4) \eqc
\end{equation}
where

- $\sigma$ is the Stefan-Boltzmann constant,
- $\epsilon$ is the emissivity of the surface,
- $F$ is the view factor function,
- $T$ is the temperature of the surface,
- $T_\infty$ is the ambient temperature, and
- $f$ is an optional scaling factor.

!syntax inputs /Components/HSBoundaryRadiation

!syntax children /Components/HSBoundaryRadiation
