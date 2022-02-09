# HeatSourceFromTotalPower

This component is a
[heat structure heat source](thermal_hydraulics/component_groups/heat_structure_heat_source.md)
from a total power $Q$, provided by a
[power component](thermal_hydraulics/component_groups/power.md). A fraction $f$
of this total power may be specified, as well as a shape function
$a(\mathbf{r})$ if a non-uniform distribution is desired.

## Usage

In addition to the parameters discussed in
[thermal_hydraulics/component_groups/heat_structure_heat_source.md],
the user is required to specify the name of a
[power component](thermal_hydraulics/component_groups/power.md) via the `power`
parameter. This power can be scaled with the parameter `power_fraction`.
If a non-uniform power distribution is desired, the parameter `power_shape_function`
may be used to specify a spatial shape function, which gets normalized internally.

## Formulation

The power density from this component is the following:

!equation
q(\mathbf{r}) = f Q \frac{a(\mathbf{r})}{\tilde{A}} \eqc

where $\tilde{A}$ denotes the discrete approximation to

!equation
A \equiv \int\limits_\Omega a(\mathbf{r}) d\Omega \eqc

where $\Omega$ is the heat source domain.
Note that the discrete integral of the power density over $\Omega$
is exactly equal to $f Q$.

!syntax parameters /Components/HeatSourceFromTotalPower

!syntax inputs /Components/HeatSourceFromTotalPower

!syntax children /Components/HeatSourceFromTotalPower
