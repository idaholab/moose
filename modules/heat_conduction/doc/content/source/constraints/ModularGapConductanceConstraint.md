# Modular Gap Conductance Constraint

!syntax description /Constraints/ModularGapConductanceConstraint

The `ModularGapConductanceConstraint` class is used specify a heat flux across a
gap.  The flux is computed by user objects derived from `GapFluxModelBase`. Such as

- [GapFluxModelSimple](GapFluxModelSimple.md)
- [GapFluxModelRadiative](GapFluxModelRadiative.md)

Multiple models can be specified with their contributions getting summed up.

!syntax parameters /Constraints/ModularGapConductanceConstraint

!syntax inputs /Constraints/ModularGapConductanceConstraint

!syntax children /Constraints/ModularGapConductanceConstraint

!bibtex bibliography
