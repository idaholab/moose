# PointInSubdomainCheckUO

`PointInSubdomainCheckUO` performs point-in-polyhedron (in-out) tests for several closed
surfaces at once and identifies which subdomain a point belongs to. It builds one
ray-casting in-out test per subdomain from the subdomain-grouped boundary elements
supplied by a
[`SurfaceMeshBySubdomainBuilder`](userobjects/SurfaceMeshBySubdomainBuilder.md). In
addition to a plain inside/outside query, it exposes `whichSubdomain`, which returns the
ID of the first subdomain whose surface contains the point (or the invalid subdomain ID
when the point lies outside every surface).

This object is the in-out tester consumed by
[`SubdomainElementModifier`](meshmodifiers/SubdomainElementModifier.md) to assign
background-mesh elements to subdomains.

## Usage

Set the `builder` parameter to the name of a `SurfaceMeshBySubdomainBuilder` user object.
The `eps` tolerance controls how close to a surface a point is treated as on it, and
`leaf_max_size` tunes the per-subdomain KDTree. Because the in-out test requires closed,
replicated surface meshes, run these tests with `mesh_mode = REPLICATED`.

!syntax description /UserObjects/PointInSubdomainCheckUO

!syntax parameters /UserObjects/PointInSubdomainCheckUO

!syntax inputs /UserObjects/PointInSubdomainCheckUO

!syntax children /UserObjects/PointInSubdomainCheckUO
