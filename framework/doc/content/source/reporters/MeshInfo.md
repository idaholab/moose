# MeshInfo

A Reporter object for tracking mesh information, such as the number of elements, nodes, or
degrees-of-freedom. The [!param](/Reporters/MeshInfo/items) parameter controls the items
computed. `MeshInfo` will declare reporters for each item requested.

## Example Input Syntax

The following input file snippet demonstrates the use of the `MeshInfo` object.

!listing mesh_info/mesh_info.i block=Reporters/mesh_info

!syntax parameters /Reporters/MeshInfo

!syntax inputs /Reporters/MeshInfo

!syntax children /Reporters/MeshInfo
