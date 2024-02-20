# MultiAppCopyTransfer

The MultiAppCopyTransfer allows for copying variables (both [nonlinear](syntax/Variables/index.md)
and [auxiliary](/AuxVariables/index.md)) between [MultiApps](/MultiApps/index.md). All types
of variables, including higher order, elemental, and nodal are supported. The only limitation is that
the meshes in the parent and sub application must be identical.

## Siblings transfer behavior

This transfer supports sending data from a MultiApp to a MultiApp if and only if the number of subapps
in the source MultiApp matches the number of subapps in the target MultiApp, and they are distributed
the same way on the parallel processes. Each source app is then matched to the target app with the same
subapp index.

## Example Syntax

!listing test/tests/transfers/multiapp_copy_transfer/linear_lagrange_to_sub/parent.i block=Transfers

!syntax parameters /Transfers/MultiAppCopyTransfer

!syntax inputs /Transfers/MultiAppCopyTransfer

!syntax children /Transfers/MultiAppCopyTransfer
