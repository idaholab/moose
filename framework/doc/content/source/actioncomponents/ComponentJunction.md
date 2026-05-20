# ComponentJunction

!syntax description /ActionComponents/ComponentJunction

The `ComponentJunction` is an [ActionComponent.md] which creates a contiguous junction
between two components.
The following [!param](/ActionComponents/ComponentJunction/junction_method) options are currently supported to join these two components at boundaries selected on each component:

- `stitch_meshes` when the two components are already in contact, forming a conformal but not yet "stitched" mesh
- `extrude_boundary` for extruding one boundary towards the other and stitching the extruded volume onto both surfaces.
  See additional information below.

There is currently no support for joining two components when the boundary meshes would not be conformal if they were
in contact.

This components inherits from the following interfaces to help facilitate the definition of equations on its geometry:

- [ComponentPhysicsInterface.md] to define [Physics](syntax/Physics/index.md)
- [ComponentMaterialPropertyInterface.md] to define functor-based material properties on that component
- [ComponentInitialConditionInterface.md] to define initial conditions for the variables defined on that component
- [ComponentBoundaryConditionInterface.md] to define boundary conditions for the variables defined on that component

## Additional details on the extrude_boundary option

To find the name of the boundaries to stitch, we recommend setting the `verbose` parameter on both the
[!param](/ActionComponents/ComponentJunction/first_component) and [!param](/ActionComponents/ComponentJunction/second_component),
and examining their intermediate mesh.


The `extrude_boundary` option has the following characteristics:

- The spline curve guiding the extrusion uses the `direction` attribute of the components to set its beginnning and end-direction.
- Thus, only components that either inherit from the `ComponentMeshTransformHelper` class or that have a `direction`
  input parameter are supported at this time.
- These directions should be pointing towards the outside of the component.
- The spline curve guiding the extrusion is created by the [BSplineCurveGenerator.md]. It connected the centroids of the
  two component boundaries.
- The extrusion of the first component surface follows this spline curve. It is then stitched to the second component surface.
- Thus, extruding one surface onto the other can only be done if the meshes of the two surfaces would be conformal if they were
  translated and rotated (using the rotation from one component direction to the other) to be in contact.

It is the responsibility of the user to check that the extruded mesh does not overlap with other parts of the mesh (from other components
for example). A [MeshDiagnosticsGenerator.md] can be connected to the final generator of the `ComponentJunction` by setting its input
to `<junction_name>_stitcher`.

!syntax parameters /ActionComponents/ComponentJunction

!syntax inputs /ActionComponents/ComponentJunction

!syntax children /ActionComponents/ComponentJunction
