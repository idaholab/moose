# SetupMeshAction

!syntax description /Mesh/SetupMeshAction

The `SetupMeshAction` is responsible for

- setting the mesh base, which may come from `MeshGenerators`, from the legacy mesh loading (`file` parameter),
  or from a split mesh, a restart or a recovery process

- the uniform refinement parameters

- conversion from first to second order mesh

- the creation of the displaced mesh

- modifications to the mesh generation process when using split meshes

- mesh initialization


More information about the `Mesh` may be found on the
[Mesh syntax documentation](syntax/Mesh/index.md).

!syntax parameters /Mesh/SetupMeshAction
