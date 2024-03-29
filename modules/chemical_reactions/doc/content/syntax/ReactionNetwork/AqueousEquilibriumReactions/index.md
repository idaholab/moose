# AqueousEquilibriumReactions syntax

This syntax, nesting parameters in the `[ReactionNetwork/AqueousEquilibriumReactions]` block, is used to describe aqueous equilibrium chemical reactions.
The following actions use the parameters defined in this syntax:

- [AddCoupledEqSpeciesAction.md] to define the kernels and auxiliary kernels for the equations describing the chemical reactions.
- [AddPrimarySpeciesAction.md] to define the nonlinear variables for the primary species.
- [AddSecondarySpeciesAction.md] to define the auxiliary variables for the secondary species.

!syntax list /ReactionNetwork/AqueousEquilibriumReactions objects=True actions=False subsystems=False

!syntax list /ReactionNetwork/AqueousEquilibriumReactions objects=False actions=False subsystems=True

!syntax list /ReactionNetwork/AqueousEquilibriumReactions objects=False actions=True subsystems=False

