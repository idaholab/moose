# ThermalSolidPropertiesMaterial

!syntax description /Materials/ThermalSolidPropertiesMaterial

## Description

The `ThermalSolidPropertiesMaterial` material is an
abstract base class from which all thermal solid materials properties should
inherit. This base class declares density, specific heat, and thermal
conductivity as material properties and computes them in
`computeQpProperties()`. Applications requiring additional thermal properties,
such as emissivity and derivatives of thermal properties with respect to
temperature, should inherit from this class and declare additional solid
properties and override `computeQpProperties` to perform the additional calculations.

## Example Input File Syntax

The `ThermalSolidPropertiesMaterial` is used in an input file as:

!listing modules/solid_properties/test/tests/stainless_steel_316/test.i block=Modules

The solid property user object is then specified as:

!listing modules/solid_properties/test/tests/stainless_steel_316/test.i block=Materials

!syntax parameters /Materials/ThermalSolidPropertiesMaterial

!syntax inputs /Materials/ThermalSolidPropertiesMaterial

!syntax children /Materials/ThermalSolidPropertiesMaterial

!bibtex bibliography
