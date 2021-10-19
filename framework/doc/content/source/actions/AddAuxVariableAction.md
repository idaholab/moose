# AddAuxVariableAction

!syntax description /AuxVariables/AddAuxVariableAction

This action derives from the [AddVariableAction.md]. It checks the auxiliary variable family and type before
adding it to the [Problem](syntax/Problem/index.md).

Auxiliary variables, which are not directly being solved for in the nonlinear system,
are specified as an object inside the `[AuxVariables]` block.

More information about auxiliary variables can be found on the [AuxVariables syntax page](syntax/AuxVariables/index.md).

!syntax parameters /AuxVariables/AddAuxVariableAction
