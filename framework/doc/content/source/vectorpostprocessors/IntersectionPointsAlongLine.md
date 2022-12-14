# IntersectionPointsAlongLine

!syntax description /VectorPostprocessors/IntersectionPointsAlongLine

The CSV output consists of the X, Y and Z coordinates of the intersection points between the faces/sides of the elements and the line.
The `IntersectionPointsAlongLine` vector postprocessor declares a vector for each coordinate of the intersection points, named `x`, `y` and `z`.

!alert note
This object only supports replicated meshes. A distributed version of this object may be created using the [ray tracing module](modules/ray_tracing/index.md optional=True).

!syntax parameters /VectorPostprocessors/IntersectionPointsAlongLine

!syntax inputs /VectorPostprocessors/IntersectionPointsAlongLine

!syntax children /VectorPostprocessors/IntersectionPointsAlongLine
