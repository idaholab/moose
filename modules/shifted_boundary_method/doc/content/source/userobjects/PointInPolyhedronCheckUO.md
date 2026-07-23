# PointInPolyhedronCheckUO

`PointInPolyhedronCheckUO` performs point-in-polyhedron (in-out) tests against a single
closed surface mesh. It wraps the boundary elements and centroids produced by an
[`SBMSurfaceMeshBuilder`](userobjects/SBMSurfaceMeshBuilder.md) in a ray-casting
in-out test that reports, for any query point, whether the point is inside, outside, or on
the surface. Points on the surface are treated as inside.

The underlying test casts a ray from the query point and counts boundary-element
crossings, accelerated by a KDTree candidate search and an oriented bounding box. When no
`ray_direction` is supplied, a robust direction is chosen automatically.

## Usage

Set the `builder` parameter to the name of an `SBMSurfaceMeshBuilder` user object. The
`eps` tolerance controls how close to the surface a point is treated as on it, and
`leaf_max_size` tunes the KDTree. Because the in-out test requires a closed, replicated
surface mesh, run these tests with `mesh_mode = REPLICATED`.

!syntax description /UserObjects/PointInPolyhedronCheckUO

!syntax parameters /UserObjects/PointInPolyhedronCheckUO

!syntax inputs /UserObjects/PointInPolyhedronCheckUO

!syntax children /UserObjects/PointInPolyhedronCheckUO
