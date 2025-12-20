# DeleteElementsNearMeshGenerator

!syntax description /Mesh/DeleteElementsNearMeshGenerator

## Implementation details

The proximity of an element to the [!param](/Mesh/DeleteElementsNearMeshGenerator/proximity_mesh)
is computed as follows. If the element centroid or any of its nodes is inside the `proximity mesh` as reported by its point locator, then the distance is zero and the element is always deleted. Else, we use heuristics to determine the distance.

A side Gauss quadrature is created on all the element external sides of the `proximity_mesh`. The quadrature points from this
quadrature are gathered into a K-nearest neighbor object.

Then this KNN object provides the distance from the `proximity_mesh` to the element by computing the distance from
the element centroid and to the element nodes. If any are below the specified distance, the element is deleted.

!syntax parameters /Mesh/DeleteElementsNearMeshGenerator

!syntax inputs /Mesh/DeleteElementsNearMeshGenerator

!syntax children /Mesh/DeleteElementsNearMeshGenerator
