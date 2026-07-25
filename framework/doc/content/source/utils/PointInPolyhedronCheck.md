# PointInPolyhedronCheck

`PointInPolyhedronCheck` is the ray-casting engine for point-in-solid queries
against a closed surface mesh. It operates on the surface-element wrappers and
centroids held by a `SurfaceElementSet`, and classifies a query point as
`SurfaceSide::INSIDE`, `::OUTSIDE`, or `::ON`.

The engine casts a ray from the query point and counts crossings of the surface
elements, accelerated by a [KDTree.md] candidate search and a projected oriented
bounding box built once at construction. The ray direction is either selected
automatically from a principal-component analysis of the surface (the `pca_ray`
method) or supplied by the caller (the `user_selected_ray` method); axis-aligned
directions use an axis-aligned bounding-box fast path. The engine supports both
2-D `EDGE2` and 3-D `TRI3` surfaces.

This engine backs the `pca_ray` and `user_selected_ray` methods of
[PointInPolyhedronCheckUO.md] through the [PointContainmentClassifier.md] facade. It
can also be used directly by C++ callers that need an arbitrary ray direction,
which is not exposed at the input-file layer.
