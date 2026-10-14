# SurfaceMeshContactor

!syntax description /UserObjects/SurfaceMeshContactor

## Description

Rigid contactor described by a closed, oriented, triangulated surface mesh
(typically an STL file).  On load the mesh is validated as a closed
2-manifold, transformed (`scale` then `translation`), and stored in a
KDTree over triangle centroids for fast nearest-triangle lookup.  Each
query computes the closest point on the nearest triangle plus a small
edge-neighbor sweep, and derives the sign of the signed distance from
the *angle-weighted pseudonormal* at the closest surface feature
following [!cite](baerentzen2005pseudonormal).  This is the choice that
gives the correct inside/outside classification at edges and vertices of
the manifold --- using a single triangle's face normal (as a naive
implementation would) is fragile at convex corners.

For the role this contactor plays in the overall formulation, see the
[rigid-body contact theory](modules/contact/rigid_contact/theory.md#level-set-contactors)
page.  The Vickers indentation
[example](modules/contact/rigid_contact/examples/material_into_indenter.md)
is the shipped end-to-end demonstration.

## Parameters

!syntax parameters /UserObjects/SurfaceMeshContactor

## References

!bibtex bibliography
