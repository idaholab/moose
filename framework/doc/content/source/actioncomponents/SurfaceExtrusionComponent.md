# SurfaceExtrusionComponent

!syntax description /ActionComponents/SurfaceExtrusionComponent

The `SurfaceExtrusionComponent` is a simple [ActionComponent.md] which is extruded from a boundary of another
component. This component is only implemented for extruding a 1D boundary into a 2D mesh or a 2D boundary into
a 3D mesh. It automatically stitches the newly created mesh onto the starting mesh.
The `SurfaceExtrusionComponent` is marked as connected to the component it joins.

The `SurfaceExtrusionComponent` relies on the [AdvancedExtruderGenerator.md] to create the mesh and notably offers:

- single-direction extrusion, using the [!param](/ActionComponents/SurfaceExtrusionComponent/direction) parameter
- radial expansion along the extrusion axis, which is formed by the extrusion direction and the centroid of the
  source surface. The source and target radial extent can be specified, as well as the radial expansion rate (derivative)
  at the source and at the end of the extrusion.

This components inherits from the following interfaces to help facilitate the definition of equations on its geometry:

- [ComponentPhysicsInterface.md] to define [Physics](syntax/Physics/index.md)
- [ComponentMaterialPropertyInterface.md] to define functor-based material properties on that component
- [ComponentInitialConditionInterface.md] to define initial conditions for the variables defined on that component
- [ComponentBoundaryConditionInterface.md] to define boundary conditions for the variables defined on that component

!syntax parameters /ActionComponents/SurfaceExtrusionComponent

!syntax inputs /ActionComponents/SurfaceExtrusionComponent

!syntax children /ActionComponents/SurfaceExtrusionComponent
