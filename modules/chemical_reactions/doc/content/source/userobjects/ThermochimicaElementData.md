# ThermochimicaElementData

!syntax description /UserObjects/ThermochimicaElementData

!alert note title=For Use with Thermochimica
This UserObject is designed for use with thermochemistry library Thermochimica.

## Description

[`ThermochimicaElementData`](ThermochimicaElementData.md) performs Thermochimica calculations on elements.
In this UserObject, the masses of elements included in the vector variable [!param](/UserObjects/ThermochimicaElementData/elements) are input
to the Fortan 90 module Thermochimica, along with the temperature and pressure. Optionally, the
user may disable Thermochimica calculation re-initialization by setting [!param](/UserObjects/ThermochimicaElementData/reinit_type) to `none`.
This may reduce memory use in the calculation, but will likely greatly increase the length of each
call to Thermochimica.

!alert note
Currently this object is designed to only work with constant monomial type variables (one DOF per element)!

Thermochimica is called by this object at every execute (please see the Thermochimica user manual
for more details), and the data required to re-initialize Thermochimica calculations is
saved/loaded if re-initialization is enabled.

If the optional variable [!param](/UserObjects/ThermochimicaElementData/output_phases) is set to a non-empty list of phase names, then concentration
data corresponding to these phases will be output. These phase names must exactly match those specified
in the datafile used for Thermochimica calculations. Similarly, if [!param](/UserObjects/ThermochimicaElementData/output_species) is a non-empty list
of `phase:species` pairs, these will be parsed and the specified species in the specified phases will be
located in Thermochimica output and stored for later output via an AuxKernel.

If [!param](/UserObjects/ThermochimicaElementData/output_element_potentials) is a non-empty list formatted as `any_string:element_name`, then the chemical
potentials of the elements in the list are output to the variables specified in the list.

## Example Input Syntax

!syntax parameters /UserObjects/ThermochimicaElementData

!syntax inputs /UserObjects/ThermochimicaElementData

!syntax children /UserObjects/ThermochimicaElementData
