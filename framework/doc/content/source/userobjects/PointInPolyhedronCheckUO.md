# PointInPolyhedronCheckUO

`PointInPolyhedronCheckUO` determines whether a point is inside, outside, or on a
single closed surface mesh. It obtains the surface mesh and the surface-element
wrappers from a [BoundaryMeshBuilder.md] named through the `builder` parameter,
and reports a per-point classification. Points on the surface are treated as
inside. The object overrides `spatialValue()` (1 for inside/on, 0 for outside),
so it works directly with [SpatialUserObjectAux.md].

## Point-containment methods

The `point_containment_method` parameter selects the classification backend:

- `pca_ray` (default) casts a ray from the query point and counts surface-element
  crossings, using a direction chosen automatically from a principal-component
  analysis of the surface. Supports 2-D `EDGE2` and 3-D `TRI3` surfaces.
- `user_selected_ray` uses the same ray-casting engine with the direction given
  by the `ray_direction` parameter. Axis-aligned directions use the
  bounding-box fast path. Supports 2-D `EDGE2` and 3-D `TRI3` surfaces.
- `fixed_x_ray` uses the [TriangleManifold.md] engine (a fixed +x ray with a
  solid-angle fallback). This engine supports only 3-D `TRI3` surfaces and does
  not produce OBB/ray debug files.

`ray_direction` is meaningful only for `user_selected_ray`; it must be non-zero
there and must be left unset for the other methods. `obb_file_name` and
`ray_file_name` request debug output from the ray-casting backends; with
`fixed_x_ray` they are ignored and an informational message is emitted.

## Usage

Set `builder` to the name of a [BoundaryMeshBuilder.md] user object. `tolerance`
controls how close to the surface a point is treated as on it, and
`leaf_max_size` tunes the KD-tree used by the ray-casting backends. Because the
in-out test requires a closed, replicated surface mesh, run these cases with
`mesh_mode = REPLICATED`.

!syntax description /UserObjects/PointInPolyhedronCheckUO

!syntax parameters /UserObjects/PointInPolyhedronCheckUO

!syntax inputs /UserObjects/PointInPolyhedronCheckUO

!syntax children /UserObjects/PointInPolyhedronCheckUO
