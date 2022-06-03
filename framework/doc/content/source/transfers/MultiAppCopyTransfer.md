# MultiAppCopyTransfer

The MultiAppCopyTransfer allows for copying variables (both [nonlinear](syntax/Variables/index.md)
and [auxiliary](/AuxVariables/index.md)) between [MultiApps](/MultiApps/index.md). All types
of variables, including higher order, elemental, and nodal are supported. The only limitation is that
the meshes in the parent and sub application must be identical.

## Example Syntax

!listing test/tests/transfers/multiapp_copy_transfer/linear_lagrange_to_sub/parent.i block=Transfers

!syntax parameters /Transfers/MultiAppCopyTransfer

!syntax inputs /Transfers/MultiAppCopyTransfer

!syntax children /Transfers/MultiAppCopyTransfer
