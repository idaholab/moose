# InterceptedElementModifier

`InterceptedElementModifier` is a `MeshModifier` that classifies each element of a
background mesh as inside or outside a geometry and assigns it one of two subdomain IDs
(`subdomain_id_inside` or `subdomain_id_outside`). The geometry is described either by a
signed-distance `Function` (via `signed_dist_function`) or by an in-out test user object
(via `in_out_test`, a [`PointInPolyhedronCheckUO`](userobjects/PointInPolyhedronCheckUO.md)).

For each element the modifier:

- classifies the element as fully inside or fully outside when all of its nodes fall on
  the same side of the geometry (using the `threshold` value for the signed-distance
  case);
- otherwise estimates, via quadrature of order `qrule_order`, the active-area fraction of
  the element that lies inside the geometry and compares `1 - fraction` against the
  `lambda` threshold to decide the assignment. Elements whose active fraction lands
  exactly on the cut are resolved with a fuzzy comparison so the classification is
  reproducible across platforms.

Setting `outer_boundary` to `true` swaps the inside/outside roles, which is convenient
when the retained domain is the region outside the surface. When `mark_intercepted` is
enabled, every partially intercepted element is instead assigned `subdomain_id_intercepted`
rather than being resolved by the `lambda` cut.

## Usage

Provide exactly one geometry source: either `signed_dist_function` or `in_out_test`.
Because the geometric in-out test requires a replicated surface mesh, run these tests with
`mesh_mode = REPLICATED`.

!syntax description /MeshModifiers/InterceptedElementModifier

!syntax parameters /MeshModifiers/InterceptedElementModifier

!syntax inputs /MeshModifiers/InterceptedElementModifier

!syntax children /MeshModifiers/InterceptedElementModifier
