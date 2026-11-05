# CavityComponent

!syntax description /ActionComponents/CavityComponent

The `CavityComponent` is a simple [ActionComponent.md] which encloses one or more other components.
Its mesh is formed from the tetrahedralization of the domain between a surface mesh, created or loaded using
mesh generators in the `[Mesh]` block, and the components.
It automatically stitches the newly created mesh onto the starting mesh.
The `CavityComponent` is marked as connected to the component it joins.

The `CavityComponent` mainly relies on the [XYZDelaunayGenerator.md] to create the mesh in 3D and the [XYDelaunayGenerator.md] in 2D.
It offers the following options:

- the target element volume (area in 2D) can be set using the [!param](/ActionComponents/CavityComponent/target_element_volume) parameter
- the enclosing mesh can be automatically recentered using a [TransformGenerator.md]'s `TRANSLATE_CENTER` option using the [!param](/ActionComponents/CavityComponent/recenter_surface_mesh_to) parameter
- extruded boundary layers can be added inside the cavity mesh using [!param](/ActionComponents/CavityComponent/n_boundary_layers) parameter.
  When the component mesh is made of hexahedrals, we need to form a transition layer to be able to stitch to the cavity's tetrahedral elements

This components inherits from the following interfaces to help facilitate the definition of equations on its geometry:

- [ComponentPhysicsInterface.md] to define [Physics](syntax/Physics/index.md)
- [ComponentMaterialPropertyInterface.md] to define functor-based material properties on that component
- [ComponentInitialConditionInterface.md] to define initial conditions for the variables defined on that component
- [ComponentBoundaryConditionInterface.md] to define boundary conditions for the variables defined on that component

!syntax parameters /ActionComponents/CavityComponent

!syntax inputs /ActionComponents/CavityComponent

!syntax children /ActionComponents/CavityComponent
