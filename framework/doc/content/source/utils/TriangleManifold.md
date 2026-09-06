# TriangleManifold

`TriangleManifold` classifies points against a closed, consistently oriented
triangulated (`TRI3`) surface mesh in 3-D. Given a prepared 2-D surface mesh it
validates that the mesh forms a closed 2-manifold, builds a lightweight yz-plane
acceleration grid, and answers point-in-solid queries.

Containment is resolved in three stages:

1. a cheap global bounding-box rejection;
2. near-surface detection via a pre-built point locator (points within
   `surface_tolerance` of the surface are treated as on the surface);
3. odd/even parity counting along a fixed +x ray, with an automatic fallback to a
   solid-angle accumulation test when the ray grazes a triangle edge or vertex.

The class exposes `contains()`, which returns true for interior and on-surface
points, and `sideness()`, which returns `SurfaceSide::INSIDE`, `::OUTSIDE`, or
`::ON`. On-surface points are detected before parity counting, so `sideness()`
and `contains()` agree (`contains() == sideness() != OUTSIDE`).

This engine backs the `fixed_x_ray` method of [PointInPolyhedronCheckUO.md]
(via [PointContainmentClassifier.md]). It supports only 3-D `TRI3` surfaces. The
referenced mesh must be serialized and must outlive the object; any geometric
transforms should be applied to the mesh before construction.
