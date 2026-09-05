# Phase Gibbs Energy Outputs

The `[PhaseGibbsEnergies]` block selects the Gibbs energy of a phase. Each leaf requires `phase` and
accepts `unit = joules` or `unit = joules_per_mole`.

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition/thermo/Outputs/PhaseGibbsEnergies

See [ChemicalCompositionAction.md#thermochemical-outputs] for output definitions and units.
