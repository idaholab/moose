# UnsignedDistanceToSurfaceMesh

`UnsignedDistanceToSurfaceMesh` is a `Function` that queries the KDTree and boundary
element database produced by [`SBMSurfaceMeshBuilder`](userobjects/SBMSurfaceMeshBuilder.md)
to provide distance information relative to an arbitrary surface mesh. The function
locates the nearest boundary element via a nearest-neighbor search in the KDTree,
computes the distance vector using the element-specific `distanceFrom` method, and
returns the vector norm as the function value. The gradient evaluates to the normalized
direction from the boundary element toward the query point.

## Usage

Set the `builder` parameter to the name of an `SBMSurfaceMeshBuilder` user object that
was configured to store the relevant surface mesh. Because the function caches pointers
to the KDTree, element ID map, and boundary element wrappers during `initialSetup`, the
builder object must be declared in the same execution block (for example inside
`[UserObjects]`) and be executed before any kernels request the function values.

!syntax description /Functions/UnsignedDistanceToSurfaceMesh

!syntax parameters /Functions/UnsignedDistanceToSurfaceMesh

!syntax inputs /Functions/UnsignedDistanceToSurfaceMesh

!syntax children /Functions/UnsignedDistanceToSurfaceMesh
