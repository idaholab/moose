# Thermochimica Inverse Nodal Data

!syntax description /UserObjects/ThermochimicaInverseNodalData

!alert note title=For Use with Thermochimica
This UserObject is designed for use with thermochemistry library Thermochimica.

## Description

[`ThermochimicaInverseNodalData`](ThermochimicaInverseNodalData.md) provides an interface to the
Thermochimia thermochemistry library to preform energy minimization calculations to determine phase
information at given state variables. The algorithm is very similar to
[`ThermochimicaNodalData`](ThermochimicaNodalData.md), except that instead of providing elemental
compositions for all elements present in the system, one (and only one) element can be solved for using
[`NestedSolve`](NestedSolve.md) to search for a elemental composition that returns a specificed
chemical potential for the particular species. This is enabled by providing a coupled variable via
the `chemical_potential` input sytax, along with an index corresponding to that particular via
`which_mu`.

Similar to [`ThermochimicaNodalData`](ThermochimicaNodalData.md),
[`ChemicalComposition`](ChemicalComposition.md) is utilized to enable this method For all other
specifications, [`ThermochimicaNodalData`](ThermochimicaNodalData.md) is should be referenced.

## Example Input Syntax

!syntax parameters /UserObjects/ThermochimicaNodalData

!syntax inputs /UserObjects/ThermochimicaNodalData

!listing modules/chemical_reactions/test/tests/thermochimia/MoRu_single_point_inverse.i block=ChemicalComposition
