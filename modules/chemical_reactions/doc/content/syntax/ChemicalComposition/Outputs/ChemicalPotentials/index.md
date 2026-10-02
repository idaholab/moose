# Chemical Potential Outputs

The `[ChemicalPotentials]` block selects a conventional solution species or an MQM thermodynamic
component. Each leaf requires `phase` and exactly one of `species`, `quadruplet`, `endmember`, or
`pair`. Results are reported in J/mol.

!listing modules/chemical_reactions/test/tests/thermochimica/typed_mqm.i block=ChemicalComposition/thermo/Outputs/ChemicalPotentials

See [ChemicalCompositionAction.md#thermochemical-outputs] for the distinction between quadruplet
and pair endmember potentials.
