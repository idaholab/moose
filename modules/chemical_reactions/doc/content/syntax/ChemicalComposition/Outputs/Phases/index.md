# Phase Outputs

The `[Phases]` block selects phase amounts from a named thermodynamic system. Each leaf requires a
`phase` and accepts `unit = moles` or `unit = mole_fraction`.

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition/thermo/Outputs/Phases

See [ChemicalCompositionAction.md#thermochemical-outputs] for output definitions and units.
