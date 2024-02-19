# ThermochimicaNodalData

!syntax description /UserObjects/ThermochimicaNodalData

!alert note title=For Use with Thermochimica
This UserObject is designed for use with thermochemistry library Thermochimica.

## Description

[`ThermochimicaNodalData`](ThermochimicaNodalData.md) performs Thermochimica calculations at nodes.

!alert note
This object can only be set up using the [ChemicalComposition](ChemicalCompositionAction.md) action

Thermochimica is called by this object at every execute (please see the Thermochimica user manual
for more details), and the data required to re-initialize Thermochimica calculations is
saved/loaded if re-initialization is enabled.

## Example Input Syntax

!syntax parameters /UserObjects/ThermochimicaNodalData

!syntax inputs /UserObjects/ThermochimicaNodalData

!syntax children /UserObjects/ThermochimicaNodalData
!syntax children /UserObjects/ThermochimicaNodalUO2X
!syntax children /UserObjects/ThermochimicaNodalUZr
