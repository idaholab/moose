# HeatSourceFromTotalPower

This component is a
[heat structure heat source](thermal_hydraulics/component_groups/heat_structure_heat_source.md)
from a total power $Q$, provided by a
[power component](thermal_hydraulics/component_groups/power.md). A fraction $f$
of this total power may be specified, as well as a shape function
$a(\mathbf{x})$ if a non-uniform distribution is desired.

## Usage

!template load file=heat_structure_heat_source_usage.md.template name=HeatSourceFromTotalPower

The user is required to specify the name of a
[power component](thermal_hydraulics/component_groups/power.md) via the
[!param](/Components/HeatSourceFromTotalPower/power)
parameter. This power can be scaled with the parameter
[!param](/Components/HeatSourceFromTotalPower/power_fraction).
If a non-uniform power distribution is desired, the parameter
[!param](/Components/HeatSourceFromTotalPower/power_shape_function)
may be used to specify a spatial shape function, which gets normalized internally.

!syntax parameters /Components/HeatSourceFromTotalPower

## Formulation

!include heat_structure_formulation.md

The power density from this component is the following:

!equation
q'''(\mathbf{x}) = f Q \frac{a(\mathbf{x})}{\tilde{A}} \eqc

where $\tilde{A}$ denotes the discrete approximation to

!equation
A \equiv \int\limits_\Omega a(\mathbf{x}) d\Omega \eqc

where $\Omega$ is the heat source domain.
Note that the discrete integral of the power density over $\Omega$
is exactly equal to $f Q$.

!syntax inputs /Components/HeatSourceFromTotalPower

!syntax children /Components/HeatSourceFromTotalPower
