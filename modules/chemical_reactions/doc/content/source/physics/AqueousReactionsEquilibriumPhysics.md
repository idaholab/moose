# AqueousReactionsEquilibriumPhysics

## Building the equation using Kernels

This [Physics](syntax/Physics/index.md) creates the equations for the mass conservation equation for each primary species
following [!cite](lichtner1996). See [chemical_reactions/index.md] for more information on the equations.

The following kernels are used:

- [CoupledBEEquilibriumSub.md] for the time derivative
- [CoupledDiffusionReactionSub.md] for the diffusion term
- [CoupledConvectionReactionSub.md] for the advection term

## Variable definition

The `AqueousReactionsEquilibriumPhysics` takes care of defining solver and auxiliary variables for the species
declared in the [!param](/Physics/AqueousReactionsEquilibrium/primary_species)
and [!param](/Physics/AqueousReactionsEquilibrium/secondary_species) parameters
respectively.

The `order` of the variables is shared across all variables.

## Reaction Network parsing

See [ReactionNetworkUtils.md] for the acceptable syntax for the
[!param](/Physics/AqueousReactionsEquilibrium/reactions) parameter describing
the reaction network.

!alert note
Only one product can be set per reaction and it should be an equilibrium species. Other products may
be moved to the left hand side (LHS). If they are equilibrium species as well, they should set as the RHS in another
equation.

The logarithm of the equilbrium constant previously specified at the end of the reaction:

```
A + B = C   2
```

is now specified as metadata between square brackets:

```
A + B -> C   [K=100]
```

!alert warning
The equilibrium constants in the legacy syntax was the `log10` of the constant. To reproduce this input,
you should specify `[log10_K=...]` instead of `[K=..]` which is now the actual constant rather than its logarithm.

Other differences:

- the equal sign `=` has been replaced by an arrow `->`.
- the comma between reactions has been replaced by a linebreak

!syntax parameters /Physics/AqueousReactionsEquilibrium/AqueousReactionsEquilibriumPhysics

!syntax inputs /Physics/AqueousReactionsEquilibrium/AqueousReactionsEquilibriumPhysics

!syntax children /Physics/AqueousReactionsEquilibrium/AqueousReactionsEquilibriumPhysics

!bibtex bibliography
