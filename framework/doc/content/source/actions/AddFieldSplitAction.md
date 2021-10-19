# AddFieldSplitAction

!syntax description /Preconditioning/AddFieldSplitAction

Field splits are specified as an object inside the `[FSP]` block, nested in the `[Preconditioning]`
block. This action adds them to the [Problem](syntax/Problem/index.md).

MOOSE can use field [splits](source/splits/Split.md) with Schur decomposition to
[precondition](syntax/Preconditioning/index.md) nonlinear systems.

!syntax parameters /Preconditioning/AddFieldSplitAction
