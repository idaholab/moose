# CombineComponentsMeshes

This [Action.md] serves to combine the meshes of each [ActionComponent.md] into a final simulation mesh.
The mesh from the `[Mesh]` block is combined with the components' meshes as well.

!alert warning
The components' meshes are currently combined with a [CombinerGenerator.md]. This means the
mesh is not stitched at the interfaces. The user must specify the connection between components
using interface conditions. These can for example be set with [boundary conditions](syntax/BCs/index.md) or dedicated [UserObjects](UserObjects/index.md).

!syntax parameters /ActionComponents/CombineComponentsMeshes

!syntax inputs /ActionComponents/CombineComponentsMeshes

!syntax children /ActionComponents/CombineComponentsMeshes
