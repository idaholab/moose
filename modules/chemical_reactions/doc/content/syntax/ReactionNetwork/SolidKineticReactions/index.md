# SolidKineticReactions syntax

This syntax, nesting parameters in the `[ReactionNetwork/SolidKineticReactions]` block, is used to describe solid kinetic reactions.
The following actions use the parameters defined in this syntax:

- [AddCoupledSolidKinSpeciesAction.md] to define the kernels and auxiliary kernels for the equations describing the chemical reactions.
- [AddPrimarySpeciesAction.md] to define the nonlinear variables for the primary species.
- [AddSecondarySpeciesAction.md] to define the auxiliary variables for the secondary species.

!syntax list /ReactionNetwork/SolidKineticReactions objects=True actions=False subsystems=False

!syntax list /ReactionNetwork/SolidKineticReactions objects=False actions=False subsystems=True

!syntax list /ReactionNetwork/SolidKineticReactions objects=False actions=True subsystems=False