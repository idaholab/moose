# SavedMeshComponent

The `SavedMeshComponent` is a geometrical component which lives on the mesh output by a mesh generator.
The component's block restriction is set to the blocks of the saved mesh, and the saved mesh
is included in the final simulation mesh.

!alert! note
There are large constraints when using a `SavedMeshComponent`:

- the final mesh generator should be specified in the `[Mesh] block (this is true for most components)
- the mesh used by a `SavedMeshComponent` should be a saved mesh, using the `save_in` parameter of mesh generators
- the mesh used should likely not be present in the `[Mesh]` block final mesh, as they would overlap with the
`SavedMeshComponent` mesh.
- the blocks on the saved mesh must have different block IDs and names from the other parts of the simulation
  mesh, as the spatial definition of the component is based on the saved mesh blocks.
  We strongly recommend using a prefix to distinguish the component's blocks' names. A [RenameBlockGenerator.md]
  can be used for that purpose.


!alert-end!

!syntax parameters /ActionComponents/SavedMeshComponent

!syntax inputs /ActionComponents/SavedMeshComponent

!syntax children /ActionComponents/SavedMeshComponent
