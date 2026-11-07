# System Property Outputs

Each leaf in `[SystemProperties]` uses `property` to select the integral system `enthalpy`,
`entropy`, or `heat_capacity`. Enthalpy is reported in J; entropy and heat capacity are reported in
J/K.

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition/thermo/Outputs/SystemProperties

Requesting this output family performs additional equilibrium solves for the temperature
derivatives used by Thermochimica. See
[ChemicalCompositionAction.md#thermochemical-outputs] for the property definitions and limitation.
