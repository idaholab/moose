# ThermalSolidPropertiesMaterial

## Description

The `ThermalSolidPropertiesMaterial` material is an
abstract base class from which all thermal solid materials properties should
inherit. This base class declares density, specific heat, and thermal
conductivity as material properties and computes them in
`computeQpProperties()`. This base class also declares the derivatives of
density, specific heat, and thermal conductivity with respect to temperature
and computes them in `computeQpProperties()`.

!bibtex bibliography
