# ThermochimicaElementData

!syntax description /UserObjects/ThermochimicaElementData

!alert note title=For Use with Thermochimica
This UserObject is designed for use with thermochemistry library Thermochimica.

## Description

[`ThermochimicaElementData`](ThermochimicaElementData.md) performs Thermochimica calculations on elements.

!alert note
This object can only be set up using the [ChemicalComposition](ChemicalCompositionAction.md) action

Thermochimica is called by this object at every execute (please see the Thermochimica user manual
for more details), and the data required to re-initialize Thermochimica calculations is
saved/loaded if re-initialization is enabled.

## Example Input Syntax

!syntax parameters /UserObjects/ThermochimicaElementData

!syntax inputs /UserObjects/ThermochimicaElementData

!syntax children /UserObjects/ThermochimicaElementData
