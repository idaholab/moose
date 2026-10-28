# Species Outputs

The `[Species]` block selects the amount of a species in a phase. Each leaf requires `phase` and
`species` and accepts `unit = moles` or `unit = mole_fraction`.

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition/thermo/Outputs/Species

See [ChemicalCompositionAction.md#thermochemical-outputs] for output definitions and units.
