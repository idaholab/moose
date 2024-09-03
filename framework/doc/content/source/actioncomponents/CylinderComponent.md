# CylinderComponent

!syntax description /ActionComponents/CylinderComponent

The `CylinderComponent` is a simple [ActionComponent.md] which adds a cylinder to
the mesh. The user selects the dimensionality of the cylinder using the [!param](/ActionComponents/CylinderComponent/dimension) parameter.

- 0D is currently unsupported
- 1D makes the cylinder a simple 1D line, with attributes to keep track of its true radius and volume
- 2D makes the cylinder a rectangle within a local 2D RZ frame of reference
- 3D is not implemented, and would create a 3D cylindrical mesh.


[Physics](Physics/index.md) can be created on this component using the [!param](/ActionComponents/CylinderComponent/physics)
parameter. This parameter accepts a vector of names of `Physics`. The name of the `Physics` is generally
found in the innermost block. For example, in the snippet below, the diffusion `Physics` is called `added_from_component`.

!listing test/tests/actioncomponents/component_with_physics.i block=Physics

The cylinder component on which the `component_with_physics` [DiffusionCG.md] is active is then
created as shown below:

!listing test/tests/actioncomponents/component_with_physics.i block=ActionComponents

!syntax parameters /ActionComponents/CylinderComponent

!syntax inputs /ActionComponents/CylinderComponent

!syntax children /ActionComponents/CylinderComponent
