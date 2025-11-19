# SBMBndElementBase

`SBMBndElementBase` wraps a libMesh surface element (`EDGE2`, `TRI3`, …) and provides the
basic geometric operations needed by the Shifted Boundary Method.  Instances of this
class are created inside [`SBMSurfaceMeshBuilder`](userobjects/SBMSurfaceMeshBuilder.md)
and are consumed by [`UnsignedDistanceToSurfaceMesh`](functions/UnsignedDistanceToSurfaceMesh.md)
when computing distances.
