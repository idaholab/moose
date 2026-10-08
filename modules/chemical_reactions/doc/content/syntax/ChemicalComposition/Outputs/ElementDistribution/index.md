# Element Distribution Outputs

The `[ElementDistribution]` block selects the amount or distribution fraction of an element in a
phase. Each leaf requires `phase` and `element` and accepts `unit = moles` or `unit = fraction`.

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition/thermo/Outputs/ElementDistribution

See [ChemicalCompositionAction.md#thermochemical-outputs] for output definitions and units.
