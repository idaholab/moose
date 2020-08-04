# Reaction paths

Notation and definitions are described in [geochemistry_nomenclature.md].

This page follows Chapters 13 and 14 of [!cite](bethke_2007).

The reaction paths that may be simulated are as follows.

- Adding reactants at a constant rate, such as adding simple reactants that are linear combinations of the basis species.

- Discarding masses of any minerals present in the equilibrium solution ("dump").

- Removing mineral masses at the end of each step (setting $n_{k}$ very small, not zero) ("flow-through")

- Adding pure H$_{2}$O and removing an equal mass of water components and solutes it contains ("flush")

- Changing temperature at a constant rate.

- Changing temperature at a constant rate by adding cooler/hotter reactants at a constant rate.

- Fixing species activity or gas fugacity over any of the above reactant paths (a basis species with fixed activity [may be](equilibrium.md) eliminated from the basis and its activity used directly in evaluating the mass-action equation, and similarly for gases, but there are subtleties associated with maintaining charge balance)

- Vary species activity or gas fugacity at a constant rate.

- Vary species log(activity) or gas log(fugacity) at a constant rate.

!bibtex bibliography
