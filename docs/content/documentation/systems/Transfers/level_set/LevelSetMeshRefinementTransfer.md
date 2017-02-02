# LevelSetMeshRefinementTransfer
When solving the level set equation performing reinitialization of the level set variable is often required
to maintain its conservative or signed distance characteristics. Within MOOSE the reinitialization step of the solve
is handled using the [MultiApp](/MultiApps/index.md) system.

To allow for the sub-application that is performing the reinitialization to use the adapted mesh of the master
application the mesh adaptivity information must be transferred from the master application to the sub-application,
this transfer is performed by the [LevelSetMeshRefinementTransfer](#).

## Example Syntax

!input modules/level_set/tests/transfers/markers/multi_level/master.i block=Transfers

!parameters /Transfers/LevelSetMeshRefinementTransfer

!inputfiles /Transfers/LevelSetMeshRefinementTransfer

!childobjects /Transfers/LevelSetMeshRefinementTransfer
