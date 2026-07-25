# PointContainmentClassifier

`PointContainmentClassifier` is a thin facade that unifies the point-containment
backends behind a single API and result type (`SurfaceSide`). Given a
builder-owned surface mesh and, for the ray-casting methods, a `SurfaceElementSet`,
it constructs exactly one backend and dispatches `sideness()` and `contains()` to
it:

- `pca_ray` and `user_selected_ray` construct a [PointInPolyhedronCheck.md]
  (ray-casting engine), differing only by the ray direction passed in;
- `fixed_x_ray` constructs a [TriangleManifold.md] engine (fixed +x ray, 3-D
  `TRI3` surfaces only).

The facade adds no geometry logic of its own beyond backend selection, the
uniform `contains()` mapping (`INSIDE` and `ON` both map to true), and routing of
the `boundingBox()` and `numElements()` accessors to the constructed backend. It
is the object owned by [PointInPolyhedronCheckUO.md]; callers never touch the
individual backends directly.
