# CreateExecutionerAction

!syntax description /Executioner/CreateExecutionerAction

This action can add a default preconditioner for each nonlinear system that does not have one
specified in the `[Preconditioning]` block.
This action examines the parameter in the `[Executioner]` block for consistency with the `Problem`.

More information about `Executioners` may be found on the
[Executioners syntax documentation](syntax/Executioner/index.md).

!syntax parameters /Executioner/CreateExecutionerAction
