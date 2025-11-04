# ComponentMaterialPropertyInterface

The `ComponentMaterialPropertyInterface` is a base class designed to facilitate the use of [Physics](Physics/index.md)
by an [ActionComponent.md]. It offers:

- the vector [!param](/ActionComponents/CylinderComponent/property_names) and [!param](/ActionComponents/CylinderComponent/property_values) parameters
  to list the material properties with these names and functor values
- the boolean [!param](/ActionComponents/CylinderComponent/define_material_properties) and [!param](/ActionComponents/CylinderComponent/define_functor_properties)
  to decide whether to define material or functor material properties or both

An [ActionComponent.md] inheriting `ComponentMaterialPropertyInterface` must be registered to the `init_component_physics`
task. For example,

!listing framework/src/actioncomponents/CylinderComponent.C start=registerMooseAction end=InputParameters

!alert note
This helper leverages virtual inheritance: it inherits the [ActionComponent.md] class virtually.
This offers some simplicity in its definition, but components leveraging this helper must also inherit
the `ActionComponent` class virtually.
