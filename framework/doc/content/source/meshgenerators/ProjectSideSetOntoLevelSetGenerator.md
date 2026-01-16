# ProjectSideSetOntoLevelSetGenerator

!syntax description /Mesh/ProjectSideSetOntoLevelSetGenerator

The projection vector defined with [!param](/Mesh/ProjectSideSetOntoLevelSetGenerator/direction)
is normalized automatically.

!alert note
The projected nodes are found using a bisection method, starting at a distance of [!param](/Mesh/ProjectSideSetOntoLevelSetGenerator/max_search_distance).
If adjusting this parameter to improve performance, you have to make sure the surface is no further than this search distance.

!alert note
It is the user's responsability to make sure the projected mesh is a valid mesh. You can use the [MeshDiagnosticsGenerator.md] to verify the
validity of the mesh automatically.

!syntax parameters /Mesh/ProjectSideSetOntoLevelSetGenerator

!syntax inputs /Mesh/ProjectSideSetOntoLevelSetGenerator

!syntax children /Mesh/ProjectSideSetOntoLevelSetGenerator

