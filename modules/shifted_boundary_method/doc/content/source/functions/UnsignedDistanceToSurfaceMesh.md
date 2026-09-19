# UnsignedDistanceToSurfaceMesh

`UnsignedDistanceToSurfaceMesh` is a `Function` that provides distance information
relative to a surface mesh. It locates the nearest surface element, returns the distance
magnitude as the function value, and returns the normalized direction from the surface
toward the query point as the gradient.

## Geometry Sources

Exactly one geometry source must be selected:

- Set `builder` to an [`SBMSurfaceMeshBuilder`](userobjects/SBMSurfaceMeshBuilder.md) for a
  saved mesh containing one interface.
- Set `manager` to an [`SBMInterfaceManager`](userobjects/SBMInterfaceManager.md), together
  with `subdomain_id_1` and `subdomain_id_2`, for an interface detected in a saved mesh
  containing the complete outward-oriented surface of each subdomain.

Both user objects construct their search data during `initialSetup` and must be declared
in `[UserObjects]`. In manager mode, reversing the two subdomain IDs reverses the surface
normal returned by `surfaceNormal`; the unsigned distance and its gradient are unchanged.

!syntax description /Functions/UnsignedDistanceToSurfaceMesh

!syntax parameters /Functions/UnsignedDistanceToSurfaceMesh

!syntax inputs /Functions/UnsignedDistanceToSurfaceMesh

!syntax children /Functions/UnsignedDistanceToSurfaceMesh
