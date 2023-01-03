# HeatSourceFromPowerDensity

This component is a
[heat structure heat source](thermal_hydraulics/component_groups/heat_structure_heat_source.md)
from a power density variable $q'''$.

## Usage

!template load file=heat_structure_heat_source_usage.md.template name=HeatSourceFromPowerDensity

The user provides a power density variable using the parameter
[!param](/Components/HeatSourceFromPowerDensity/power_density).

!syntax parameters /Components/HeatSourceFromPowerDensity

## Formulation

!include heat_structure_formulation.md

!syntax inputs /Components/HeatSourceFromPowerDensity

!syntax children /Components/HeatSourceFromPowerDensity
