# HSBoundaryHeatFlux

This component is a
[heat structure boundary](thermal_hydraulics/component_groups/heat_structure_boundary.md)
that applies a specified heat flux function on the boundary.

## Usage

!template load file=heat_structure_boundary_usage.md.template name=HSBoundaryHeatFlux

The parameter [!param](/Components/HSBoundaryHeatFlux/q) gives the incoming
boundary heat flux function $q$.

The parameter [!param](/Components/HSBoundaryHeatFlux/scale_pp) specifies
the name of a post-processor $f$ that can scale the boundary conditions.

!syntax parameters /Components/HSBoundaryHeatFlux

## Formulation

!include heat_structure_formulation.md

!include heat_structure_boundary_formulation_neumann.md

This incoming boundary flux is the product of the user-specified incoming
boundary flux function $q$ and the optional scaling factor $f$:

!equation
q_b = f q \eqp

!syntax inputs /Components/HSBoundaryHeatFlux

!syntax children /Components/HSBoundaryHeatFlux
