# Vapor Pressure Outputs

The `[VaporPressures]` block selects a species partial pressure from a gas phase. Each leaf requires
`phase` and `species`; the result uses the configured system pressure unit.

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition/thermo/Outputs/VaporPressures

See [ChemicalCompositionAction.md#thermochemical-outputs] for output definitions and units.
