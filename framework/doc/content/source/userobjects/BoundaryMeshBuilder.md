# BoundaryMeshBuilder

`BoundaryMeshBuilder` owns a saved surface (boundary) mesh and the surface-element
wrappers built from it, exposing both for reuse by point-containment and
distance user objects. It is the sole retriever of the saved mesh: a mesh saved
through a [MeshGenerator](syntax/Mesh/index.md)'s `save_with_name` parameter has
single-retrieval semantics, so exactly one object may own it. Consumers hold a
non-owning reference to this builder rather than retrieving the saved mesh again.

## Description

The builder retrieves the saved surface mesh named by the `surface_mesh`
parameter during `initialSetup`, prepares it for use, and validates that:

- the mesh is replicated (serialized); a distributed surface mesh is rejected,
  because point-containment queries require the whole surface on every rank;
- the background (embedding) mesh dimension equals the surface mesh dimension
  plus one (e.g. a 2-D surface inside a 3-D domain).

The surface-element wrappers (the `SurfaceElementSet`) are built lazily on first
use, so backends that classify from the mesh directly (such as the `fixed_x_ray`
[TriangleManifold.md] engine) pay no allocation for a set they never read.

When `check_watertightness = true` the builder reports, via an informational
message, whether every surface element has a neighbor on every side. A
non-watertight surface may not be suitable for in-out tests.

## Example Input Syntax

```
[UserObjects]
  [surface_builder]
    type = BoundaryMeshBuilder
    surface_mesh = boundary_mesh
  []
[]
```

!syntax description /UserObjects/BoundaryMeshBuilder

!syntax parameters /UserObjects/BoundaryMeshBuilder

!syntax inputs /UserObjects/BoundaryMeshBuilder

!syntax children /UserObjects/BoundaryMeshBuilder
