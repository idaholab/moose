# ThermalStainlessSteel316Properties

!syntax description /Modules/SolidProperties/ThermalStainlessSteel316Properties

## Description

`ThermalStainlessSteel316Properties` provides thermal properties for
stainless steel 316 as a function of temperature.

Isobaric specific heat, thermal conductivity, and density as calculated as
curve fits of tabulated data in [cite:mills]. Molar mass is calculated assuming the
nominal composition in [cite:mills]. Emissivity is calculated based on whether
the surface is oxidized or polished according to data in [cite:mills].

## Range of Validity

The ThermalStainlessSteel316Properties UserObject is valid for
25 $\degree$C $\le$ T $\le$ 1300 $\degree$C.

## Example Input File Syntax

!listing modules/solid_properties/test/tests/stainless_steel_316/test.i block=Modules

!syntax parameters /Modules/SolidProperties/ThermalStainlessSteel316Properties

!syntax inputs /Modules/SolidProperties/ThermalStainlessSteel316Properties

!syntax children /Modules/SolidProperties/ThermalStainlessSteel316Properties

!bibtex bibliography
