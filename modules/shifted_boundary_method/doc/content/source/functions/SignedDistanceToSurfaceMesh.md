# SignedDistanceToSurfaceMesh

`SignedDistanceToSurfaceMesh` is a `Function` that returns the signed distance to an
arbitrary surface mesh. It extends
[`UnsignedDistanceToSurfaceMesh`](functions/UnsignedDistanceToSurfaceMesh.md) by
multiplying the unsigned nearest-boundary distance by a sign obtained from an in-out test
user object (via `in_out_test`, a
[`PointInPolyhedronCheckUO`](userobjects/PointInPolyhedronCheckUO.md)): the value is
negative inside the surface and positive outside. As with the unsigned function, the
gradient evaluates to the unit vector pointing from the boundary toward the query point.

## Usage

Set the `builder` parameter to the `SBMSurfaceMeshBuilder` user object that stores the
surface mesh, and `in_out_test` to a `PointInPolyhedronCheckUO` built from the same
surface. Both user objects must be declared in the same execution block (for example
inside `[UserObjects]`) and executed before any kernels request the function values.

!syntax description /Functions/SignedDistanceToSurfaceMesh

!syntax parameters /Functions/SignedDistanceToSurfaceMesh

!syntax inputs /Functions/SignedDistanceToSurfaceMesh

!syntax children /Functions/SignedDistanceToSurfaceMesh
