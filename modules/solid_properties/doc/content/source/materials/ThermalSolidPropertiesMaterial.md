# ThermalSolidPropertiesMaterial

## Description

The `ThermalSolidPropertiesMaterial` material declares
density, specific heat, and thermal
conductivity as material properties and computes them in
`computeQpProperties()`.

!alert warning title=Conservation on fixed-sized domains
Using a variable density can lead to mass/energy conservation errors if using
a fixed-size domain. If this is a concern, it is recommended to use
[ConstantDensityThermalSolidPropertiesMaterial.md] instead, which uses a constant
density.

!syntax parameters /Materials/ThermalSolidPropertiesMaterial

!syntax inputs /Materials/ThermalSolidPropertiesMaterial

!syntax children /Materials/ThermalSolidPropertiesMaterial

!bibtex bibliography
