# DistanceAux

`DistanceAux` evaluates the unsigned distance from an element centroid to the closest
surface described by a [`ShortestDistanceToSurface`](../userobjects/ShortestDistanceToSurface.md)
user object. The auxiliary kernel calculates the Euclidean distance from an element's
centroid to the boundary represented by the `distance_to_surface` object.

## Usage

Provide the name of a `ShortestDistanceToSurface` user object through the
`distance_to_surface` parameter. The user object can be backed by either level-set
functions or an [`SBMSurfaceMeshBuilder`](../userobjects/SBMSurfaceMeshBuilder.md)
if the distance should be measured to a boundary represented by a mesh file.

!syntax description /AuxKernels/DistanceAux

!syntax parameters /AuxKernels/DistanceAux

!syntax inputs /AuxKernels/DistanceAux

!syntax children /AuxKernels/DistanceAux
