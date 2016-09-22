# MultiAppCopyTransfer

!devel /Transfers/MultiAppCopyTransfer float=right width=auto margin=20px padding=20px background-color=#F8F8F8

!description /Transfers/MultiAppCopyTransfer

## Description
The MultiAppCopyTransfer allows for copying variables (both [nonlinear](auto::/Variables/Overview) and [auxiliary](auto::/AuxVariables/Overview)) between [MultiApps](auto::/MultiApps/Overview). All types of
variables, including higher order, elemental, and nodal are supported. The only limitiation is that the
meshes in the master and sub application must be identical.

## Example Syntax
!input test/tests/transfers/multiapp_copy_transfer/linear_lagrange_to_sub/master.i block=Transfers

!parameters /Transfers/MultiAppCopyTransfer

!inputfiles /Transfers/MultiAppCopyTransfer

!childobjects /Transfers/MultiAppCopyTransfer
