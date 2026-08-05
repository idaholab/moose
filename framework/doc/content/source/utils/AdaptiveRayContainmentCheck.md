# AdaptiveRayContainmentCheck

`AdaptiveRayContainmentCheck` is the ray-casting engine for point-in-solid queries
against a closed surface mesh. It operates on the surface-element wrappers and
centroids held by a `SurfaceElementSet`, and classifies a query point as
`SurfaceSide::INSIDE`, `::OUTSIDE`, or `::ON`.

The engine casts a ray from the query point and counts crossings of the surface
elements, accelerated by a [KDTree.md] candidate search. It follows one of two
explicit ray-direction policies:

- `AUTO_PCA` (the `pca_ray` method): the direction is chosen automatically from a
  principal-component analysis of the surface, an oriented bounding box is built
  once at construction, and, on a parity tie, the engine probes the remaining
  variance directions.
- `USER_SPECIFIED` (the `user_selected_ray` method): the caller's direction is used
  exactly, with no automatic selection or fallback, over the global axis-aligned
  bounding box. Any finite, non-zero direction is allowed, including oblique ones;
  for a 2-D surface it must lie in the mesh plane. A genuinely ambiguous
  (grazing/tangent) query is reported as an error rather than silently redirected.

The engine supports both 2-D `EDGE2` and 3-D `TRI3` surfaces.

This engine backs the `pca_ray` and `user_selected_ray` methods of
[PointInPolyhedronCheckUO.md] through the [PointContainmentClassifier.md] facade.
