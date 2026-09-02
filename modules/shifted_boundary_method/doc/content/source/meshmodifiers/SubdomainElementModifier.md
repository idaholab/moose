# SubdomainElementModifier

`SubdomainElementModifier` is a `MeshModifier` that assigns each element of a background
mesh a subdomain ID based on geometric inclusion in one or more closed surfaces. The
per-subdomain in-out tests are provided by a [`PointInSubdomainCheckUO`](userobjects/PointInSubdomainCheckUO.md),
which in turn is populated from a [`SurfaceMeshBySubdomainBuilder`](userobjects/SurfaceMeshBySubdomainBuilder.md)
that groups a surface (boundary) mesh into one closed loop per subdomain.

For each element the modifier:

- assigns the element to a subdomain if all of the element's nodes lie inside that
  subdomain's surface (the lowest such subdomain ID wins when an element is fully inside
  more than one);
- otherwise estimates, via quadrature, the active-area fraction inside each surface the
  element intercepts and assigns the element to the surface with the largest fraction
  (ties broken by lowest subdomain ID);
- leaves the element's current subdomain unchanged when it is outside every surface, or
  when the largest active-area fraction is below the `lambda` threshold.

## Usage

Set `subdomain_id_tester` to a `PointInSubdomainCheckUO`. Use `lambda` to control how much
of an intercepted element must fall inside a surface before it is reassigned, and set
`mark_intercepted` together with `subdomain_id_intercepted` to place every intercepted
element into a single dedicated subdomain instead. Because the underlying in-out test
requires a replicated surface mesh, run these tests with `mesh_mode = REPLICATED`.

!syntax description /MeshModifiers/SubdomainElementModifier

!syntax parameters /MeshModifiers/SubdomainElementModifier

!syntax inputs /MeshModifiers/SubdomainElementModifier

!syntax children /MeshModifiers/SubdomainElementModifier
