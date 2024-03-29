# ReactionNetwork syntax

The `ReactionNetwork` parent block syntax is shared by the [aqueous equilibrium reactions](AqueousEquilibriumReactions/index.md)
and [solid kinetics reactions](SolidKineticReactions/index.md) syntaxes. Both are nested under their respective sub-block, and can be
defined simultaneously.

!syntax list /ReactionNetwork objects=True actions=False subsystems=False

!syntax list /ReactionNetwork objects=False actions=False subsystems=True

!syntax list /ReactionNetwork objects=False actions=True subsystems=False