# ChemicalComposition Outputs

## Description

The `[Outputs]` block within a named `[ChemicalComposition]` system contains typed selections for
phases, species, element potentials, vapor pressures, and element distributions among phases. Each
leaf block creates one scalar auxiliary variable using the leaf-block name, unless its `variable`
parameter supplies another name.

See [ChemicalCompositionAction.md] for the parameters required by each output family and the
deprecated flat output parameters.

## Example Input Syntax

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition/thermo/Outputs
