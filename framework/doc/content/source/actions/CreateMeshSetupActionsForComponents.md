# CreateMeshSetupActionsForComponents

!syntax description /ActionComponents/CreateMeshSetupActionsForComponents

This `Action` creates the [SetupMeshAction.md] and the [SetupMeshCompleteAction.md] when the
`[ActionComponents]` syntax is used. These two actions are normally only created if the `[Mesh]`
syntax is used.

!alert note
To select specific `Mesh` operations, such as adding a partitioner or selecting the mesh parallel
type, a `[Mesh]` block should be used, in which case, this `Action` does not perform any operation.

!syntax parameters /ActionComponents/CreateMeshSetupActionsForComponents
