# CylinderComponent

!syntax description /ActionComponents/CylinderComponent

The `CylinderComponent` is a simple [ActionComponent.md] which adds a cylindrical component to
the mesh. The user selects the dimensionality of the cylinder using the [!param](/ActionComponents/CylinderComponent/dimension) parameter. For example, the `Cylinder` is
a simple line in 1D, with attributes to keep track of its true radius and volume. It is a rectangle with a local 2D RZ frame of
reference in 2D, and a true cylindrical mesh (not implemented) in 3D.

[Physics](Physics/index.md) can be created on this component using the [!param](/ActionComponents/CylinderComponent/physics)
parameter. This parameter accepts a vector of names of `Physics`. The name of the `Physics` is generally
found in the innermost block. For example, in the snippet below, the diffusion `Physics` is called `diff`.

!listing test/tests/actioncomponents/cylinder.i block=Physics

The cylinder component on which the `diff` [DiffusionPhysicsCG.md] is active is then
created as shown below:

!listing test/tests/actioncomponents/cylinder.i block=Components

!syntax parameters /ActionComponents/CylinderComponent

!syntax inputs /ActionComponents/CylinderComponent

!syntax children /ActionComponents/CylinderComponent
