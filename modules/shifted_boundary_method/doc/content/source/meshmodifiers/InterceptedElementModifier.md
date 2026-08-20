# InterceptedElementModifier

`InterceptedElementModifier` is a `MeshModifier` that classifies each element of a
background mesh as inside or outside a geometry and assigns it one of two subdomain IDs
(`subdomain_id_inside` or `subdomain_id_outside`). The geometry is described either by a
signed-distance `Function` (via `signed_dist_function`) or by an in-out test user object
(via `in_out_test`, a [`PointInPolyhedronCheckUO`](PointInPolyhedronCheckUO.md)).

For each element the modifier:

- classifies the element as fully inside or fully outside only when all of its nodes fall
  on the same side of the geometry (using the `threshold` value for the signed-distance
  case) *and* the quadrature estimate of the active-area fraction agrees (fully active or
  fully inactive). Requiring both guards against a surface that crosses the element or
  encloses a small region even though every node lies on one side;
- otherwise estimates, via quadrature of order `qrule_order`, the active-area fraction of
  the element that lies inside the retained domain and compares `1 - fraction` against the
  `lambda` threshold to decide the assignment. Elements whose active fraction lands
  exactly on the cut are resolved with a fuzzy comparison so the classification is
  reproducible across platforms.

`is_domain_inside_surface` selects which side of the surface is the retained (inside)
domain. Set it to `true` when the retained domain is the region *enclosed by* the surface
(signed distance below `threshold`, or points reported inside by the in-out test), and to
`false` when the retained domain is the region *outside* the surface. When
`mark_intercepted` is enabled, every partially intercepted element is instead assigned
`subdomain_id_intercepted` rather than being resolved by the `lambda` cut.

## Usage

Provide exactly one geometry source: either `signed_dist_function` or `in_out_test`.
Supplying both, or neither, is an error. Because the geometric in-out test requires a
replicated surface mesh, run these tests with `mesh_mode = REPLICATED`.

!syntax description /MeshModifiers/InterceptedElementModifier

!syntax parameters /MeshModifiers/InterceptedElementModifier

!syntax inputs /MeshModifiers/InterceptedElementModifier

!syntax children /MeshModifiers/InterceptedElementModifier
