# SetupMeshCompleteAction

!syntax description /Mesh/SetupMeshCompleteAction

This action is ran after the mesh has been setup to ensure the mesh is ready for simulation.
It takes care of deleting unnecessary remote elements, of triggering uniform
refinement and general preparation of the mesh (neighbor search, renumbering, partitioning).

!syntax parameters /Mesh/SetupMeshCompleteAction
