# SBMUtils

`SBMUtils` is a lightweight toolbox that provides the basic geometric operations
needed by the Shifted Boundary Method (SBM). It ensures that higher-level objects
can use the same simple and consistent logic.

The utilities help answer two key questions:
**“How far is this point from the boundary?”** and
**“What is the outward normal at the projected boundary point?”**

## Distance Calculations

`SBMUtils` offers easy-to-use helpers to:

- Load and validate distance functions (level-set or mesh-based).
- Compute distance vectors from any point to the nearest boundary.
- Compute outward normals using either level-set gradients or mesh geometry.
- Compare multiple surfaces and automatically pick the closest one.

These functions provide a clear and unified interface for retrieving **distance**
and **normal direction** information.

## Mesh Validation

For mesh-based boundaries, `SBMUtils` includes simple checks to confirm that the
surface mesh is well-connected and watertight before being used for spatial queries.
This supports tools like `SBMSurfaceMeshBuilder` by ensuring reliable boundary quality.

!syntax description utils/SBMUtils

!syntax parameters utils/SBMUtils

!syntax inputs utils/SBMUtils

!syntax children utils/SBMUtils
