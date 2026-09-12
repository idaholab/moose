# Constituent Fraction Outputs

Each leaf in `[ConstituentFractions]` selects a `constituent` on a phase `sublattice`. Sublattice
numbers are one-based. The resulting dimensionless auxiliary variable contains the constituent or
site fraction reported by the selected phase model. An absent phase produces a value of zero.

!listing modules/chemical_reactions/test/tests/thermochimica/typed_mqm.i block=ChemicalComposition/thermo/Outputs/ConstituentFractions

See [ChemicalCompositionAction.md#thermochemical-outputs] for supported phase models and output
behavior.
