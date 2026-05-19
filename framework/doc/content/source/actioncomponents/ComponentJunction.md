# ComponentJunction

!syntax description /ActionComponents/ComponentJunction

The `ComponentJunction` is an [ActionComponent.md] which creates a contiguous junction
between two components.
The following methods are currently supported to join these two components at boundaries selected on each component:

- `stitch_meshes` when the two components are already in contact, forming a conformal but not yet "stitched" mesh
- `extrude_boundary` for extruding one boundary towards the other and stitching the extruded volume onto both surfaces.
  This can only be done if the two surfaces have the same meshes. A spline curve following the direction of the components
  guides this extrusion.

There is currently no support for joining two components when the boundary meshes would not be conformal if they were
in contact.

This components inherits from the following interfaces to help facilitate the definition of equations on its geometry:

- [ComponentPhysicsInterface.md] to define [Physics](syntax/Physics/index.md)
- [ComponentMaterialPropertyInterface.md] to define functor-based material properties on that component
- [ComponentInitialConditionInterface.md] to define initial conditions for the variables defined on that component
- [ComponentBoundaryConditionInterface.md] to define boundary conditions for the variables defined on that component


!syntax parameters /ActionComponents/ComponentJunction

!syntax inputs /ActionComponents/ComponentJunction

!syntax children /ActionComponents/ComponentJunction
