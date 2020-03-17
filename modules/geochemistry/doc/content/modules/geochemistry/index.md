# The geochemistry module

The `geochemistry` module is designed to solve reactive transport in geochemical systems.  The `geochemistry` module is currently in development.  The documentation describes the module's functionality, but does not currently include any description of the MOOSE implementation.

The `geochemistry` module's functionality is a subset of that described in [!cite](bethke_2007).  Various aspects have not yet been included, such as the virial Pitzer/HMW models for activity.  The `geochemistry` module is currently focussed on simulation of "GeoTES" systems, which operate between between $25^{\circ}$C and $300^{\circ}$C.

The `geochemistry` module consists of two broad parts: the reactions and the transport.

## Reactions

- [An introduction to equilibrium reactions, equilibrium constants, etc](equilibrium_reactions.md)
- [The database](database.md)
- [Equilibrium in aqueous solutions, sorption, mathematical equations and the numerical solution procedure](equilibrium.md)
- [Kinetics](kinetics.md)
- [Activity coefficients](activity_coefficients.md) and [gas fugacity](fugacity.md)
- [The chemical component basis](basis.md) and [basis swaps](swap.md)
- [Reaction paths available in the `geochemistry` module](reaction_paths.md)
- [Definitions of chemistry terminology and notation used](geochemistry_nomenclature.md)

## Transport

- [Transport in the `geochemistry` module](transport.md)

## Tests and Examples

Although the MOOSE code is incomplete, some [documentation of future `geochemistry` tests](tests_and_examples/geochemistry_tests_and_examples.md) has been written.

## Implementation

- [Database reader](geochemistry_database_reader.md)





!bibtex bibliography
