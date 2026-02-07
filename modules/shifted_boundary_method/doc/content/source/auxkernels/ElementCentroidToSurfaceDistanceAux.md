# ElementCentroidToSurfaceDistanceAux

`ElementCentroidToSurfaceDistanceAux` outputs the distance from an element centroid to the closest
surface calculated by a [`ShortestDistanceToSurface`](userobjects/ShortestDistanceToSurface.md)
user object.

## Usage

Provide the name of a `ShortestDistanceToSurface` user object through the
`distance_to_surface` parameter. The user object can be backed by either level-set
functions or an [`SBMSurfaceMeshBuilder`](userobjects/SBMSurfaceMeshBuilder.md)
if the distance should be measured to a boundary represented by a mesh file.

!syntax description /AuxKernels/ElementCentroidToSurfaceDistanceAux

!syntax parameters /AuxKernels/ElementCentroidToSurfaceDistanceAux

!syntax inputs /AuxKernels/ElementCentroidToSurfaceDistanceAux

!syntax children /AuxKernels/ElementCentroidToSurfaceDistanceAux
