# AddFieldSplitAction

!syntax description /Preconditioning/AddFieldSplitAction

Field splits are specified as an object inside the `[FSP]` block, nested in the `[Preconditioning]`
block.

MOOSE can use field [splits](source/splits/Split.md) with Schur decomposition to
[precondition](syntax/Preconditioning/index.md) non-linear systems.

!syntax parameters /Preconditioning/AddFieldSplitAction
