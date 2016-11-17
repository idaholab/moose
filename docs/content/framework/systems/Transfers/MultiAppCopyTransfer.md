# MultiAppCopyTransfer


!description /Transfers/MultiAppCopyTransfer

## Description
The MultiAppCopyTransfer allows for copying variables (both [nonlinear](/Variables/Overview.md) and [auxiliary](/AuxVariables/Overview.md)) between [MultiApps](/MultiApps/Overview.md). All types of
variables, including higher order, elemental, and nodal are supported. The only limitiation is that the
meshes in the master and sub application must be identical.

## Example Syntax
!input test/tests/transfers/multiapp_copy_transfer/linear_lagrange_to_sub/master.i block=Transfers

!parameters /Transfers/MultiAppCopyTransfer

!inputfiles /Transfers/MultiAppCopyTransfer

!childobjects /Transfers/MultiAppCopyTransfer
