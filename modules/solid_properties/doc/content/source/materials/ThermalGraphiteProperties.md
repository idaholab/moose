# ThermalGraphiteProperties

!syntax description /Materials/ThermalGraphiteProperties

## Description

`ThermalGraphiteProperties` provides thermal properties for graphite
as a function of temperature. This form of
graphite is formed by baking at high temperature, and should not be used for
pyrolitic carbons formed by chemical vapor deposition.
The thermal properties of graphite are strongly dependent on the raw materials,
manufacturing process, and oxidation state, so manufacturer-specific properties,
if available, are preferred to the use of this material.

Isobaric specific heat is calculated from [cite:butland]. Thermal
conductivity is calculated from a curve fit of tabulated data in [cite:mceligot]
for G-348 isotropic graphite.
Density is calculated given a room temperature density and an estimation of an
average thermal expansion coefficient from 20$\degree$C to the temperature of
interest according to thermal expansion data in [cite:baker].
Molar mass is calculated assuming pure elemental graphite.

## Range of Validity

The ThermalGraphiteProperties material is valid for estimating isobaric
specific heat over 200 K $\le$ T $\le$ 3500 K; for estimating thermal
conductivity over 25$\degree$C $\le$ T $\le$ 1000$\degree$C; and for
estimating density over 20$\degree$C $\le$ T $\le$ 2500$\degree$C,
though the error in density estimation increases with
temperature due to the assumption of a constant thermal expansion coefficient.

## Example Input File Syntax

!listing modules/solid_properties/test/tests/graphite/test.i block=Materials

!syntax parameters /Materials/ThermalGraphiteProperties

!syntax inputs /Materials/ThermalGraphiteProperties

!syntax children /Materials/ThermalGraphiteProperties

!bibtex bibliography
