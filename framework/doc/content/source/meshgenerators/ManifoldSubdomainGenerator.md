# ManifoldSubdomainGenerator

!syntax description /Mesh/ManifoldSubdomainGenerator

## Overview

`ManifoldSubdomainGenerator` assigns a subdomain ID based on whether an element's vertex-average
point lies inside a closed surface mesh (a manifold). The surface must be watertight: every triangle
must have exactly three neighboring triangles, all elements must be Tri3, and the mesh must be
consistently oriented. Vertex-average points that lie on the surface within the configured tolerance
are treated as inside.

The manifold is supplied as a mesh via the MeshGenerator pipeline, not as a raw file path. To use an
STL file, load it with [FileMeshGenerator](FileMeshGenerator.md) and apply any needed transforms with
[TransformGenerator](TransformGenerator.md) before passing the result as
[!param](/Mesh/ManifoldSubdomainGenerator/manifold).

## Example

The example below loads a cube STL as the manifold, translates it into the mesh domain, and assigns
subdomain `1` to the elements whose vertex-average points lie inside the closed surface.

!listing test/tests/meshgenerators/manifold_subdomain/basic.i block=Mesh

Transforms are composed by chaining mesh generators. The listing above shows a scale step, a rotate
step, and a translate step in sequence. Users may also limit reassignment to selected existing blocks
with [!param](/Mesh/ManifoldSubdomainGenerator/restricted_subdomains), or tag elements that lie
outside the manifold instead by setting
[!param](/Mesh/ManifoldSubdomainGenerator/location) to `OUTSIDE`.

[!param](/Mesh/ManifoldSubdomainGenerator/surface_tolerance) is an absolute tolerance. Set it
relative to the manifold length scale and the coordinate noise produced by the mesh export pipeline.
If coincident vertices differ by more than this tolerance, the manifold validation step can reject an
otherwise visually closed surface as non-watertight.

This generator uses `Elem::vertex_average()` as its representative point, not the true geometric
centroid. That keeps the classification inexpensive and matches the sampling convention used by
similar subdomain tagging generators.

!syntax parameters /Mesh/ManifoldSubdomainGenerator

!syntax inputs /Mesh/ManifoldSubdomainGenerator

!syntax children /Mesh/ManifoldSubdomainGenerator
