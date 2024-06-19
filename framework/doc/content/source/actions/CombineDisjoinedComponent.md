# CombineDisjoinedComponent

This [Action.md] serves to combine the meshes of each component into a final simulation mesh.
The mesh from the `[Mesh]` block is combined with the components' meshes as well.

!alert warning
The components' meshes are currently combined with a [CombinerGenerator.md]. This means the
mesh is not stitched at the interfaces. The user must specify the connection between components
using either [boundary conditions](BCs/index.md) or dedicated [UserObjects](UserObjects/index.md).

!syntax parameters /ActionComponents/CombineDisjoinedComponent

!syntax inputs /ActionComponents/CombineDisjoinedComponent

!syntax children /ActionComponents/CombineDisjoinedComponent
