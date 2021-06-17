# GeochemicalModelDefinition

Defines the geochemical system.  This extracts information from the geochemistry database and stores information pertinent to the model in computationally-efficient datastructures, eliminating all extraneous information.

!alert note
All `geochemistry` models must contain a `GeochemicalModelDefinition` userobject

This userobject extensively uses `utils/PertinentGeochemicalSystem`, and it is useful to fully discuss the input parameters of this class.

## basis_species

This is a list of [basis components](basis.md) relevant to the [aqueous equilibrium problem](equilibrium.md).  The following requirements must be satisfied (otherwise a mooseError is produced and the simulation stops):

- "H2O" must appear first in this list;
- No member must appear more than once in this list;
- These components must be chosen from the "basis species" in the [database](geochemistry/database/index.md), the [sorbing sites](equilibrium.md) (if any) and the decoupled redox states that are in [disequilibrium](basis.md) (if any).

Any [redox pair](basis.md) that is not in this list or the [`kinetic_redox`](theory/index.md) list, will be assumed to be at equilibrium with the aqueous solution and will be considered a [secondary species](basis.md).

All these species, except H2O, may be later swapped out of this list, either by a manual user-prescribed [swap](swap.md) (and replaced by a mineral or a gas of fixed fugacity, for instance), or during the numerical solve.  For simple examples of this swap, see [GeochemicalModelInterrogator](GeochemicalModelInterrogator.md)

## minerals

This list contains the minerals that are in equilibrium with the aqueous solution.  Conditions on the members of this list are:

- They must be "minerals" in the [database](geochemistry/database/index.md);
- No member can appear more than once in this list;
- Their [equilibrium reaction](equilibrium.md) must consist of only the `basis_species`, and [secondary species](basis.md) and non-kinetically-controlled [redox couples](basis.md) that can be expressed in terms of the basis_species;
- If they are also "sorbing minerals" in the database then their [sorption sites](equilibrium.md) must consist of the basis_species only.

During simulation, the user can compute the [saturation index](geochemistry_nomenclature.md) of these minerals.

These minerals can be [swapped](swap.md) into the basis if desired (or required during the numerical solve).  For simple examples of this swap, see [GeochemicalModelInterrogator](GeochemicalModelInterrogator.md).  If the user performs a manual "swap" then an initial condition must be provided for the mineral.

The user choose whether these minerals are allowed to precipitate or not --- that is, they can be "supressed".

This list, along with the kinetic_minerals list, comprises the entire list of minerals in the problem: all others are eliminated from consideration.

## gases

This is a list of [gases](basis.md) that are in equilibrium with the aqueous solution and can have
their [fugacities](fugacity.md) fixed, at least at some time and spatial location.

- All members of this list must be a "gas" in the [database file](geochemistry/database/index.md).
- No gas must appear more than once in this list.
- The equilibrium reaction of each gas must involve only the basis_species, or secondary species or non-kinetically-controlled redox couples that can be expressed in terms of the basis_species.

This list comprises all the gases that are in the simulation: all others are eliminated.

## kinetic_minerals

This is a list of minerals that whose dynamics are governed by a [rate law](theory/index.md).  These are not in equilibrium with the aqueous solution.

- The list can only include the "minerals" in the [database file](geochemistry/database/index.md).
- No member can appear more than once in this list.
- The [equilibrium reaction](equilibrium.md) of each mineral must involve only the basis_species, or secondary species or non-kinetically-controlled redox couples that can be expressed in terms of the basis_species.
- If a mineral is also a "sorbing mineral" in the [database](geochemistry/database/index.md) then their [sorption sites](equilibrium.md) must consist of the basis_speices only.
- No members of this list must be in the minerals list.

These minerals can never be [swapped](swap.md) into the basis, nor can they be "supressed".


## kinetic_redox

This is a list of [redox pairs](basis.md) that whose dynamics are governed by a [rate law](theory/index.md).  These are not in equilibrium with the aqueous solution.

- The list can only include the "redox couples" in the [database file](geochemistry/database/index.md).
- No member can appear more than once in this list.
- The [equilibrium reaction](equilibrium.md) of each member must involve only the basis_species, or secondary species or non-kinetically-controlled redox couples that can be expressed in terms of the basis_species.
- No members of this list must be in the basis_species list.

These species can never be [swapped](swap.md) into the basis.

## kinetic_surface_species

This is a list of [surface sorbing species](basis.md) that whose dynamics are governed by a [rate law](theory/index.md).  These are not in equilibrium with the aqueous solution.

- The list can only include the "surface species" in the [database file](geochemistry/database/index.md).
- No member can appear more than once in this list.
- The [equilibrium reaction](equilibrium.md) of each member must involve only the basis_species, or secondary species or non-kinetically-controlled redox couples that can be expressed in terms of the basis_species.

These species can never be [swapped](swap.md) into the basis.

## kinetic_rate_descriptions

A list of [GeochemistryKineticRate](GeochemistryKineticRate.md) user objects that define the kinetic rates for the kinetic species should be supplied.  If a kinetic species has no rate prescribed then its reaction rate will be zero.  Multiple [GeochemistryKineticRate](GeochemistryKineticRate.md) user objects can apply to a single kinetic species: in this case the sum of all the rates defines the overall reaction rate for the kinetic species.

## Secondary species

The complete list of [secondary species](equilibrium.md) is automatically computed based on the above information using the following algorithm:

1. All "redox couples" in the [database](geochemistry/database/index.md) are queried one-by-one, and included if:

- they are not part of the `kinetic_redox` list; and
- they are not part of the `basis_species` list; and
- their reaction involves only basis_species or secondary species already encountered.  This means redox couples whose reactions involve already-encountered redox couples can be included in the secondary species' list.

2. All secondary species in the [database](geochemistry/database/index.md) are queried one-by-one, and included if:

- their reaction involves basis_species, or secondary species already encountered.

3. All surface species in the [database](geochemistry/database/index.md) are queried one-by-one, and included if:

- they are not in the kinetic_surface_species list; and
- their reaction involves only basis_species or secondary species encountered so far

The resulting list contains only species whose equilibrium reactions can ultimately be expressed in terms of basis species.  Once this list is created, the equilibrium reactions for the minerals, gases, kinetic_redox and kinetic_surface_species can also be checked that they can be ultimately expressed in terms of basis species as specified above in the conditions "the [equilibrium reaction](equilibrium.md) of each member must involve only the basis_species, or secondary species or non-kinetically-controlled redox couples that can be expressed in terms of the basis_species."

## Creating the pertinent model

All the aforementioned species are included in the "pertinent model" created by this class, including their names, charges, molecular weights, ionic radii, molecular volumes, stoichiometric coefficients and equilibrium constants.

## Executing

All computations in this Userobject are performed in its constructor during initial setup.  During simulation, this Userobject does nothing.

## Example

An example that simply outputs equilibrium reactions involving the clinoptilolite mineral is:

!listing modules/geochemistry/test/tests/interrogate_reactions/clinoptilolite.i





!syntax parameters /UserObjects/GeochemicalModelDefinition

!syntax inputs /UserObjects/GeochemicalModelDefinition

!syntax children /UserObjects/GeochemicalModelDefinition
